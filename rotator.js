let events = [];
// Define arguments_ and thisProgram *before* the Module object for Emscripten to pick them up.
var initialArguments = [];
var programName = "./game.html"; // Or derive from window.location.pathname

initialArguments.push(programName); // argv[0]

// --- Perform detection logic immediately ---
let desiredWidth, desiredHeight, desiredShapes, desiredColumns;

const urlParams = new URLSearchParams(window.location.search);
const isWide = urlParams.get("wide") === "true";
const isThin = urlParams.get("thin") === "true";

function getDeviceType() {
  const ua = navigator.userAgent;
  const isIPad =
    /iPad/i.test(ua) ||
    (navigator.platform === "MacIntel" && navigator.maxTouchPoints > 1);
  const isIPhone = /iPhone/i.test(ua) && !isIPad;

  if (isIPhone) {
    return "iPhone";
  } else if (isIPad) {
    return "iPad";
  } else {
    return "Other";
  }
}

const device = getDeviceType();

const isIPad = device === "iPad";

console.log(`Device type ${device}`);

if (isThin) {
  console.log("URL parameter 'thin=true' detected. Forcing thin mode.");
  desiredWidth = 550;
  desiredHeight = 900;
  desiredShapes = 6;
  desiredColumns = 2;
} else if (isWide) {
  console.log("URL parameter 'wide=true' detected. Forcing wide mode.");
  desiredWidth = 1200;
  desiredHeight = 600;
  desiredShapes = 12;
  desiredColumns = 4;
} else if (isIPad) {
  console.log("iPad detected. Using wide mode.");
  desiredWidth = 1024;
  desiredHeight = 768;
  desiredShapes = 12;
  desiredColumns = 4;
} else {
  console.log("Defaulting to thin mode.");
  desiredWidth = 550;
  desiredHeight = 900;
  desiredShapes = 6;
  desiredColumns = 2;
}
// --- End detection logic ---

let hapticLabel = null;

const detectiOS = () => {
  if (typeof navigator === "undefined") return false;
  return /iPhone|iPad|iPod/i.test(navigator.userAgent);
};

function initHaptic() {
  if (hapticLabel || typeof document === "undefined") return;

  const input = document.createElement("input");
  input.type = "checkbox";
  input.id = "es6-haptic-switch";
  input.setAttribute("switch", "");
  input.style.display = "none";
  document.body.appendChild(input);

  const label = document.createElement("label");
  label.htmlFor = "es6-haptic-switch";
  label.style.display = "none";
  document.body.appendChild(label);

  hapticLabel = label;
}

function triggerHaptic(duration = 100) {
  if (!hapticLabel) {
    console.warn("Haptic feedback not initialized. Call initHaptic() first.");
    return;
  }

  if (detectiOS()) {
    hapticLabel.click();
  } else if (navigator?.vibrate) {
    navigator.vibrate(duration);
  } else {
    hapticLabel.click(); // Fallback
  }
}

console.log(
  "Initial arguments for Module: width=" +
    desiredWidth +
    ", height=" +
    desiredHeight +
    ", shapes=" +
    desiredShapes +
    ", columns=" +
    desiredColumns,
);

initialArguments.push("--width", String(desiredWidth));
initialArguments.push("--height", String(desiredHeight));
initialArguments.push("--shapes", String(desiredShapes));
initialArguments.push("--columns", String(desiredColumns));

// Emscripten runtime will look for this global variable.
// It's also good to set it on Module.arguments for some versions/configs.
var arguments_ = initialArguments;

initHaptic();

var Module = {
  canvas: (function () {
    return document.getElementById("canvas");
  })(),
  thisProgram: programName,
  arguments: initialArguments, // Provide the initial value on Module
  preRun: [
    function () {
      // preRun can still be used for other setup tasks if needed,
      // but argument setup is now done before Module is fully processed by game.js
      console.log("Module preRun executing.");
      console.log("Arguments being passed to C++ main:", Module.arguments); // Log what C++ will receive
    },
  ],
  onRuntimeInitialized: function () {
    console.log("WASM Runtime Initialized (main has been called).");
    //processGameEvents();
    document.getElementById("canvas").addEventListener("touchend", () => {
      console.log("click");
      console.log(JSON.stringify(events));
      const eventName = events.shift();
      if (eventName === "select") {
        console.log("triggering");
        triggerHaptic();
      }
    });
  },
};
// TODO: add sounds now that I have event passing from the C++ side
function handleGameEvent(eventName) {
  console.log("JS: Handling event - " + eventName);
  if (eventName === "success") {
    if (typeof window.triggerSuccessSound === "function") {
      window.triggerSuccessSound();
    }
  } else if (eventName === "fail") {
    if (typeof window.triggerFailSound === "function") {
      window.triggerFailSound();
    }
  } else if (eventName === "select") {
    triggerHaptic();
  }
  // Add more event types as needed
}

// Leaving the full event handler just in case, but it does not interact well with having to force select
/*
function processGameEvents() {
  while (events.length > 0) {
    const eventName = events.shift(); // Get the oldest event
    if(eventName === "select"){
      continue
    }
    handleGameEvent(eventName);
  }
  requestAnimationFrame(processGameEvents); // Loop continuously
}
*/
