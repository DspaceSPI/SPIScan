#!/usr/bin/env python

import sys,os,re

class Error(Exception):
    pass


class ParseError(Error):
    def __init__(self, errline):
        Error.__init__(self, errline)


class Struct:
    pass


def createCNameMap():
    t = ''
    for i in range(256):
        if ((ord('A') <= i) and (i <= ord('Z'))) or \
               ((ord('a') <= i) and (i <= ord('z'))) or \
               ((ord('0') <= i) and (i <= ord('9'))):
            t += chr(i)
        else:
            t += '_'
    return t


def seekBegin(f):
    while True:
        line = f.readline()
        if not line:
            return False
        if line.startswith('BEGIN SANE_Option_Descriptor'):
            return True


def parseVerbatim(o, line):
    words = line.split(None, 1)
    if (len(words) < 2) or (words[1][0] != '@'):
        return False
    o[words[0]] = words[1]
    return True


def parseLine_type(o, line):
    words = line.split(None, 2)
    otype = words[1]
    o['type'] = 'SANE_TYPE_' + otype.upper()
    if otype == 'group':
        g.ngroups += 1
        oname = '_group_%d' % g.ngroups
        o['size'] = 0
    else:
        temp = words[2]
        idx = temp.find('[')
        if idx == -1:
            oname = temp
            o['size'] = 1
        else:
            oname = temp[0:idx]
            o['size'] = int(temp[idx+1:-1])
    o['name'] = oname


def parseLine_title(o, line):
    o['title'] = line.split(None, 1)[1]


def parseLine_desc(o, line):
    o['desc'] = line.split(None, 1)[1]


def parseLine_unit(o, line):
    o['unit'] = 'SANE_UNIT_' + line.split(None, 1)[1].upper()


def parseLine_default(o, line):
    o['default'] = line.split(None, 1)[1]


def parseLine_cap(o, line):
    words = line.split()
    o['cap'] = ['SANE_CAP_' + s.upper() for s in words[1:]]


def parseLine_constraint(o, line):
    c = line.split(None,1)[1]
    if c[0] == '{':
        o['constraint'] = c[1:-1].split('|')
    elif c[0] == '(':
        o['constraint'] = tuple(c[1:-1].split(','))
    else:
        sys.stderr.write('Ignored: %s\n' % line)


def parseLine_info(o, line):
    words = line.split()
    o['info'] = ['SANE_INFO_' + s.upper() for s in words[1:]]

def parseLine_rem(o, line):
    pass

def normalize(o):
    if 'cname' not in o:
        cname = o['name'].translate(cnameMap)
        o['cname'] = cname
    else:
        cname = o['cname']
    o['cname_opt'] = 'opt_' + cname
    o['cname_con'] = 'constraint_' + cname
    if 'title' not in o:
        o['title'] = 'NO TITLE'
    if 'desc' not in o:
        o['desc'] = '@sod->title' % o
    if 'unit' not in o:
        o['unit'] = 'SANE_UNIT_NONE'
    if 'constraint_type' not in o:
        if 'constraint' not in o:
            ct = 'SANE_CONSTRAINT_NONE'
        elif isinstance(o['constraint'], list):
            if o['type'] == 'SANE_TYPE_STRING':
                ct = 'SANE_CONSTRAINT_STRING_LIST'
            else:
                ct = 'SANE_CONSTRAINT_WORD_LIST'
        elif isinstance(o['constraint'], tuple):
            ct = 'SANE_CONSTRAINT_RANGE'
        elif isinstance(o['constraint'], str):
            oc = o['constraint']
            if oc.startswith('@range'):
                ct = 'SANE_CONSTRAINT_RANGE'
            elif oc.startswith('@word_list'):
                ct = 'SANE_CONSTRAINT_WORD_LIST'
            elif oc.startswith('@string_list'):
                ct = 'SANE_CONSTRAINT_STRING_LIST'
        o['constraint_type'] = ct
    return o


def parseFile(f):
    if not seekBegin(f):
        return None
    options = [ {
        'name' : '',
        'cname' : 'opt_num_opts',
        'title' : '@SANE_TITLE_NUM_OPTIONS',
        'desc' : '@SANE_DESC_NUM_OPTIONS',
        'type' : 'SANE_TYPE_INT',
        'unit' : 'SANE_UNIT_NONE',
        'size' : 1,
        'cap' : ['SANE_CAP_SOFT_DETECT'],
        'constraint_type' : 'SANE_CONSTRAINT_NONE',
        'default' : '@w = ' + opt_prefix + 'last'
        } ]
    o = {}
    while True:
        line = f.readline()
        if not line:
            break
        line = line.strip()
        if not line:
            continue
        token = line.split(None, 1)[0].lower()
        if token == 'end':
            break
        if token == 'type':
            if 'name' in o:
                options.append(o)
            o = {}
        funcName = 'parseLine_' + token
        if funcName in globals():
            if not parseVerbatim(o, line):
                func = globals()[funcName]
                func(o, line)
        else:
            sys.stderr.write('Skip: %s\n' % line)
    if 'name' in o:
        options.append(o)
    return [normalize(o) for o in options]


def genHeader(options):
    print """
typedef union {
    SANE_Word w;
    SANE_Int  i;
    SANE_Bool b;
    SANE_Fixed f;
    SANE_String s;
    void *ptr;
} option_value_t;
"""
    print 'typedef enum {'
    for o in options:
        if o['type'] == 'SANE_TYPE_GROUP':
            sys.stdout.write('\t')
        print '\t%(cname_opt)s,' % o
    print '\t\t' + opt_prefix + 'last'
    print '} option_t;'
    print """
typedef struct {
    SANE_Option_Descriptor sod;
    option_value_t val,def;
    SANE_Word info;
} option_descriptor_t;

struct pixma_sane_t;
static int build_option_descriptors(struct pixma_sane_t *ss);
"""


def genMinMaxRange(n, t, r):
    if t == 'SANE_TYPE_FIXED':
        r = ['SANE_FIX(%s)' % x for x in r]
    print 'static const SANE_Range ' + n + ' = '
    print '\t{ ' + r[0] + ',' + r[1] + ',' + r[2] + ' };'


def genList(n, t, l):
    if t == 'SANE_TYPE_INT':
        etype = 'SANE_Word'
        l = [str(len(l))] + l
    elif t == 'SANE_TYPE_FIXED':
        etype = 'SANE_Word'
        l = [str(len(l))] + ['SANE_FIX(%s)' % x for x in l]
    elif t == 'SANE_TYPE_STRING':
        etype = 'SANE_String_Const'
        l = ['SANE_I18N("%s")' % x for x in l] + ['NULL']
    print 'static const %s %s[%d] = {' % (etype, n, len(l))
    for x in l[0:-1]:
        print '\t' + x + ','
    print '\t' + l[-1] + ' };'


def genConstraints(options):
    for o in options:
        if 'constraint' not in o: continue
        c = o['constraint']
        oname = o['cname_con']
        otype = o['type']
        if isinstance(c, tuple):
            genMinMaxRange(oname, otype, c)
        elif isinstance(c, list):
            genList(oname, otype, c)
    print

def buildCodeVerbatim(o):
    for f in ('name', 'title', 'desc', 'type', 'unit', 'size', 'cap',
              'constraint_type', 'constraint', 'default'):
        if (f not in o): continue
        temp = o[f]
        if (not isinstance(temp,str)) or \
           (len(temp) < 1) or (temp[0] != '@'):
            continue
        o['code_' + f] = temp[1:]

def ccode(o):
    buildCodeVerbatim(o)
    if 'code_name' not in o:
        o['code_name'] = '"' + o['name'] + '"'
    for f in ('title', 'desc'):
        cf = 'code_' + f
        if cf in o: continue
        o[cf] = 'SANE_I18N("' + o[f] + '")'

    for f in ('type', 'unit', 'constraint_type'):
        cf = 'code_' + f
        if cf in o: continue
        o[cf] = o[f]

    if 'code_size' not in o:
        otype = o['type']
        osize = o['size']
        if otype == 'SANE_TYPE_STRING':
            code = str(osize + 1)
        elif otype == 'SANE_TYPE_INT' or otype == 'SANE_TYPE_FIXED':
            code = str(osize) + ' * sizeof(SANE_Word)'
        elif otype == 'SANE_TYPE_BUTTON':
            code = '0'
        else:
            code = 'sizeof(SANE_Word)'
        o['code_size'] = code

    if ('code_cap' not in o) and ('cap' in o):
        o['code_cap'] = reduce(lambda a,b: a+'|'+b, o['cap'])
    else:
        o['code_cap'] = '0'

    if ('code_info' not in o) and ('info' in o):
        o['code_info'] = reduce(lambda a,b: a+'|'+b, o['info'])
    else:
        o['code_info'] = '0'

    if ('code_default' not in o) and ('default' in o):
        odefault = o['default']
        otype = o['type']
        if odefault == '_MIN':
            rhs = 'w = sod->constraint.range->min'
        elif odefault == '_MAX':
            rhs = 'w = sod->constraint.range->max'
        elif otype in ('SANE_TYPE_INT', 'SANE_TYPE_BOOL'):
            rhs = 'w = %(default)s'
        elif otype == 'SANE_TYPE_FIXED':
            rhs = 'w = SANE_FIX(%(default)s)'
        elif otype == 'SANE_TYPE_STRING':
            rhs = 's = SANE_I18N("%(default)s")'
        o['code_default'] = rhs % o
    if 'code_default' in o:
        code = '\topt->def.%(code_default)s;\n'
        if o['constraint_type'] != 'SANE_CONSTRAINT_STRING_LIST':
            code += '\topt->val.%(code_default)s;\n'
        else:
            code += '\topt->val.w = find_string_in_list' \
                    '(opt->def.s, sod->constraint.string_list);\n'
        o['full_code_default'] = code % o
    else:
        o['full_code_default'] = ''

    if ('code_constraint' not in o) and ('constraint' in o):
        ct = o['constraint_type']
        idx = len('SANE_CONSTRAINT_')
        ctype = ct[idx:].lower()
        if ctype == 'range':
            rhs = '&%(cname_con)s' % o
        else:
            rhs = '%(cname_con)s' % o
        o['code_constraint'] = ctype + ' = ' + rhs
    if 'code_constraint' in o:
        code = '\tsod->constraint.%(code_constraint)s;\n'
        o['full_code_constraint'] = code % o
    else:
        o['full_code_constraint'] = ''

    return o

def genBuildOptions(options):
    print """
static
int find_string_in_list(SANE_String_Const str, const SANE_String_Const *list)
{
    int i;
    for (i = 0; list[i] && strcmp(str, list[i]) != 0; i++) {}
    return i;
}

static
int build_option_descriptors(struct pixma_sane_t *ss)
{
    SANE_Option_Descriptor *sod;
    option_descriptor_t *opt;

    memset(OPT_IN_CTX, 0, sizeof(OPT_IN_CTX));"""

    for o in options:
        o = ccode(o)
        otype = o['type']
        code = '\n\topt = &(OPT_IN_CTX[%(cname_opt)s]);\n' \
               '\tsod = &opt->sod;\n' \
               '\tsod->type = %(code_type)s;\n' \
               '\tsod->title = %(code_title)s;\n' \
               '\tsod->desc = %(code_desc)s;\n'
        if otype != 'SANE_TYPE_GROUP':
            code += '\tsod->name = %(code_name)s;\n' \
                    '\tsod->unit = %(code_unit)s;\n' \
                    '\tsod->size = %(code_size)s;\n' \
                    '\tsod->cap  = %(code_cap)s;\n' \
                    '\tsod->constraint_type = %(code_constraint_type)s;\n' \
                    '%(full_code_constraint)s' \
                    '\tOPT_IN_CTX[%(cname_opt)s].info = %(code_info)s;\n' \
                    '%(full_code_default)s'
        sys.stdout.write(code % o)
    print
    print '\treturn 0;\n'
    print '}'
    print

g = Struct()
g.ngroups = 0
opt_prefix = 'opt_'
con_prefix = 'constraint_'
cnameMap = createCNameMap()
options = parseFile(sys.stdin)
print "/* Automatically generated from pixma_sane.c */"
if (len(sys.argv) == 2) and (sys.argv[1] == 'h'):
    genHeader(options)
else:
    genConstraints(options)
    genBuildOptions(options)

