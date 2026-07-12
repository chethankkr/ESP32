document.addEventListener('DOMContentLoaded', function() {
  const controlIP = "192.168.59.82";
  const motionIP = "192.168.59.227";
  const streamURL = `http://${controlIP}/stream`;
  const stream = document.getElementById("stream");
  const statusDot = document.getElementById("status-dot");
  const motion = document.getElementById("motion-section");
  const toggleBtn = document.getElementById("toggle-btn");
  const metalAlert = document.getElementById("metal-section");
  const staticEffect = document.querySelector('.crt-static');
  const flashBtn = document.getElementById("flash-btn");
  let isFlashOn = false;

  const fetchMetalStatus = async () => {
    try {
      const response = await fetch(`http://${motionIP}/metal`);
      if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
      const text = await response.text();
      
      // const metalAlert = document.getElementById("metal-section");
      metalAlert.textContent = text === "Metal Detected" ? "DETECTED" : "NO METAL";
      metalAlert.classList.toggle('active', text === "Metal Detected");
    } catch (err) {
      console.error('Failed to fetch metal status:', err);
      // const metalAlert = document.getElementById("metal-section");
      metalAlert.textContent = "ERROR";
      metalAlert.classList.remove('active');
    }
  };

  let isStreaming = false;
  let lastFrameTime = 0;
  let noDataTimeout;

  const sendMoveCommand = async (cmd) => {
    try {
      const response = await fetch(`http://${controlIP}/control?cmd=${cmd}`);
      if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    } catch (err) {
      console.error('Failed to send command:', err);
    }
  };

  const sendCamCommand = async (cmd) => {
    try {
      const response = await fetch(`http://${motionIP}/servo?cmd=${cmd}`);
      if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    } catch (err) {
      console.error('Failed to send camera command:', err);
    }
  };

  const captureImage = async () => {
    try {
      const response = await fetch(`http://${controlIP}/capture`);
      if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
      const blob = await response.blob();
      
      const url = URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = `esp32-capture-${Date.now()}.jpg`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    } catch (err) {
      console.error("Image capture failed:", err);
      alert("Failed to capture image.");
    }
  };

  const checkStreamData = () => {
    const currentTime = Date.now();
    if (isStreaming && (currentTime - lastFrameTime) > 10000) { // 10 seconds
      // No frame for 10 seconds
      statusDot.style.backgroundColor = "red";
      staticEffect.style.display = 'block';
      toggleBtn.classList.remove('playing');
    }
  };

  const checkStreamConnection = () => {
    if (!stream.complete || stream.naturalHeight === 0) {
      statusDot.style.backgroundColor = "red";
      staticEffect.style.display = 'block';
      toggleBtn.classList.remove('playing');
    }
  };

  const streamActive = () => {
    lastFrameTime = Date.now();
    const stream = document.getElementById("stream");
    if (stream.complete && stream.naturalHeight > 0) {
      statusDot.style.backgroundColor = "green";
      toggleBtn.classList.add('playing');
      staticEffect.style.display = 'none';
    } else {
      statusDot.style.backgroundColor = "red";
      staticEffect.style.display = 'block';
      toggleBtn.classList.remove('playing');
    }
    
    // Clear any existing timeout
    if (noDataTimeout) {
      clearInterval(noDataTimeout);
    }
    // Start monitoring for no data
    noDataTimeout = setInterval(checkStreamData, 1000);
  };

  const toggleStream = () => {
    isStreaming = !isStreaming;
    if (isStreaming) {
      console.log("Starting stream...");
      stream.src = streamURL + "?t=" + new Date().getTime(); // Add timestamp to prevent caching
      lastFrameTime = Date.now();
      // Start monitoring for no data
      noDataTimeout = setInterval(checkStreamData, 1000);
      // Check connection status after a short delay
      setTimeout(checkStreamConnection, 1000);
    } else {
      console.log("Stopping stream...");
      stream.src = "";
      staticEffect.style.display = 'block';
      statusDot.style.backgroundColor = "red";
      toggleBtn.classList.remove('playing');
      // Clear the no data monitoring
      if (noDataTimeout) {
        clearInterval(noDataTimeout);
      }
    }
    toggleBtn.innerHTML = isStreaming ? '<i class="fas fa-stop"></i>' : '<i class="fas fa-play"></i>';
    toggleBtn.title = isStreaming ? "Stop Stream" : "Start Stream";
  };

  const streamError = () => {
    console.log("Stream error detected");
    statusDot.style.backgroundColor = "red";
    staticEffect.style.display = 'block';
    toggleBtn.classList.remove('playing');
    // Toggle the toggleBtn to show play icon and 'Start Stream' title
    toggleBtn.innerHTML = '<i class="fas fa-play"></i>';
    toggleBtn.title = "Start Stream";
    // Clear the no data monitoring
    if (noDataTimeout) {
      clearInterval(noDataTimeout);
    }

    if (isStreaming) {
      // Try to reconnect after 5 seconds
      setTimeout(() => {
        if (isStreaming) {
          console.log("Attempting to reconnect...");
          stream.src = streamURL + "?t=" + new Date().getTime(); // Add timestamp to prevent caching
          // Check connection status after a short delay
          setTimeout(checkStreamConnection, 1000);
        }
      }, 5000);
    }
  };

  // Add event listeners for stream
  stream.onload = streamActive;
  stream.onerror = streamError;
  stream.onabort = streamError;

  // Initialize with static effect showing
  staticEffect.style.display = 'block';
  stream.src = "";  // Start with empty stream
  statusDot.style.backgroundColor = "red";
  toggleBtn.innerHTML = '<i class="fas fa-play"></i>';
  toggleBtn.title = "Start Stream";

  const fetchPIR = async () => {
    try {
        const response = await fetch(`http://${motionIP}/pir`);
        if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
        const text = await response.text();
        
        const isMotionDetected = text === "Motion detected";
        motion.innerHTML = isMotionDetected ? "DETECTED" : "NO MOTION";
        motion.classList.toggle('active', isMotionDetected);

        // Auto start stream and capture image on motion detection
        if (isMotionDetected && !isStreaming) {
            console.log("Motion detected - starting stream and capturing image");
            
            // Simulate a button press to properly update the UI
            toggleBtn.click();

            // Capture image after a short delay to allow the stream to stabilize
            setTimeout(() => {
                captureImage();
            }, 2000); // Delay to ensure stream has started
        }
    } catch (err) {
        console.error('Failed to fetch PIR status:', err);
        motion.textContent = "ERROR";
        motion.classList.remove('active');
    }
};


  const fetchGPS = () => {
    fetch(`http://${motionIP}/gpsdata`)
      .then(res => res.json())
      .then(data => {
        const lat = data.latitude;
        const lng = data.longitude;

        document.getElementById("gps-lat").textContent = lat.toFixed(6);
        document.getElementById("gps-lng").textContent = lng.toFixed(6);
        document.getElementById("gps-alt").textContent = data.altitude.toFixed(2);
        document.getElementById("gps-speed").textContent = data.speed.toFixed(2);
        document.getElementById("gps-time").textContent = data.time;

        updateMap(lat, lng);
      })
      .catch(() => {
        ["gps-lat", "gps-lng", "gps-alt", "gps-speed", "gps-time"].forEach(id => {
          document.getElementById(id).textContent = "--";
        });
      });
  };

  const speedControl = document.getElementById('speedControl');
  const speedValue = document.getElementById('speedValue');

  const updateSpeed = async () => {
    try {
      const value = speedControl.value;
      const percentage = Math.round((value / 255) * 100);
      speedValue.textContent = `${percentage}%`;
      // Add animation class
      speedValue.classList.add('changed');
      setTimeout(() => {
        speedValue.classList.remove('changed');
      }, 300);
      // Send speed value to ESP32
      const response = await fetch(`http://${motionIP}/speed?value=${value}`);
      if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    } catch (err) {
      console.error('Failed to update speed:', err);
    }
  };

  // Update percentage live as the slider moves
  speedControl.addEventListener('input', updateSpeed);
  // Also update on change (for keyboard or accessibility)
  speedControl.addEventListener('change', updateSpeed);

  // Leaflet map setup
  let map = L.map('map').setView([0, 0], 15);
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; OpenStreetMap contributors'
  }).addTo(map);
  let marker = L.marker([0, 0]).addTo(map);

  const updateMap = (lat, lng) => {
    marker.setLatLng([lat, lng]);
    map.setView([lat, lng]);
  };

  // Initialize speed
  updateSpeed();

  // Start the periodic checks
  setInterval(fetchMetalStatus, 1000);
  setInterval(fetchPIR, 1000);
  setInterval(fetchGPS, 10000);
  setInterval(streamActive, 1000);
  fetchMetalStatus();
  fetchPIR();
  fetchGPS();

  const toggleFlash = async () => {
    // Optimistically update UI
    isFlashOn = !isFlashOn;
    flashBtn.classList.toggle('active', isFlashOn);
    flashBtn.title = isFlashOn ? "Turn Off Flash" : "Turn On Flash";
    const value = isFlashOn ? 1 : 0;
    try {
      const response = await fetch(`http://${controlIP}/control?cmd=${value ? "ON" : "OFF"}`);
      if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
    } catch (err) {
      // Revert UI if request fails
      isFlashOn = !isFlashOn;
      flashBtn.classList.toggle('active', isFlashOn);
      flashBtn.title = isFlashOn ? "Turn Off Flash" : "Turn On Flash";
      console.error('Failed to toggle flash:', err);
    }
  };

  window.toggleStream = toggleStream;
  window.captureImage = captureImage;
  window.sendMoveCommand = sendMoveCommand;
  window.sendCamCommand = sendCamCommand;
  window.toggleFlash = toggleFlash;
});