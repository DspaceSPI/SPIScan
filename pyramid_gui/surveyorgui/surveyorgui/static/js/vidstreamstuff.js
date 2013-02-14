/* Copyright (C) 2007 Richard Atterer, richardÂ©atterer.net
       * This program is free software; you can redistribute it and/or modify it
       * under the terms of the GNU General Public License, version 2. See the file
       * COPYING for details.
       */

var imageNr = 0; // Serial number of current image
var finished = new Array(); // References to img objects which have finished downloading
var paused = false;
var previous_time = new Date();

      function createImageLayer() {
        var img = new Image();
        img.style.position = "absolute";
        img.style.zIndex = -1;
        img.onload = imageOnload;
        img.onclick = imageOnclick;
        img.width = 512;
        img.height = 384;
        img.src = "/?action=snapshot&n=" + (++imageNr);
        var webcam = document.getElementById("deploycam");
        webcam.insertBefore(img, webcam.firstChild);
      }

      // Two layers are always present (except at the very beginning), to avoid flicker
      function imageOnload() {
        this.style.zIndex = imageNr; // Image finished, bring to front!
        while (1 < finished.length) {
          var del = finished.shift(); // Delete old image(s) from document
          del.parentNode.removeChild(del);
        }
        finished.push(this);
        current_time = new Date();
        delta = current_time.getTime() - previous_time.getTime();
        fps   = (1000.0 / delta).toFixed(3);
        document.getElementById('info').firstChild.nodeValue = delta + " ms (" + fps + " fps)";
        previous_time = current_time;
        if (!paused) createImageLayer();
      }

      function imageOnclick() { // Clicking on the image will pause the stream
        paused = !paused;
        if (!paused) createImageLayer();
      }


