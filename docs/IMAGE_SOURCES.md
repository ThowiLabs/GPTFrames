# Source images and licenses

Wikimedia Commons: search using action=query and generator=search, namespace File, prop=imageinfo and extmetadata. Results include page title, author, license and license URL. Download from verified https://upload.wikimedia.org or https://thumb.wikimedia.org only; allow PNG/JPEG and store attribution sidecar JSON. Check each original file's current license and attribution requirements before publishing.

Openverse: search via https://api.openverse.org/v1/images/ and preserve source, creator, license and landing page. Results may contain CC BY-NC, ND or other usage restrictions, and externally hosted URLs. GPTFrames does not download arbitrary Openverse URLs to avoid SSRF or license mistakes. Download to projects only through the Commons-specific tool.

Example: query 'cat' from Wikimedia, choose a File: title and download it as asset 'gato'. The compositor layer references assets/gato.jpg or assets/gato.png. The sidecar JSON must accompany attribution documentation for the final video.
