const CACHE_NAME = "rotator-cache-v0.0.5";
const urlsToCache = [
  "./",
  "./index.html",
  "./rotator.js",
  "./game.js",
  "./game.wasm",
  "./game.data",
  "./manifest.json",
  "./icon.png",
  "./icon192.png",
];

// Install event: opens a cache and adds the core files to it.
self.addEventListener("install", function (event) {
  event.waitUntil(
    caches.open(CACHE_NAME).then(function (cache) {
      console.log("Opened cache");
      return cache.addAll(urlsToCache);
    }),
  );
});

// Fetch event: serves assets from cache if available, otherwise fetches from network.
self.addEventListener("fetch", function (event) {
  event.respondWith(
    caches.match(event.request).then(function (response) {
      // Cache hit - return response
      if (response) {
        return response;
      }
      return fetch(event.request);
    }),
  );
});

// Activate event: cleans up old caches.
self.addEventListener("activate", function (event) {
  const cacheWhitelist = [CACHE_NAME];
  event.waitUntil(
    caches.keys().then(function (cacheNames) {
      return Promise.all(
        cacheNames.map(function (cacheName) {
          if (cacheWhitelist.indexOf(cacheName) === -1) {
            return caches.delete(cacheName);
          }
        }),
      );
    }),
  );
});
