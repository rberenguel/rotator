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

const isIPad =
  /iPad/i.test(navigator.userAgent) ||
  (navigator.platform === "MacIntel" &&
    navigator.maxTouchPoints > 1 &&
    !window.MSStream);

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
    processGameEvents();
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
  }
  // Add more event types as needed
}

function processGameEvents() {
  while (events.length > 0) {
    const eventName = events.shift(); // Get the oldest event
    handleGameEvent(eventName);
  }
  requestAnimationFrame(processGameEvents); // Loop continuously
}