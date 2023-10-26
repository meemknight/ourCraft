# ourCraft

It is the third time i try to make minecraft from scratch.
This time I want to add a lot of harder to implement features like transparent blocks, lights shadows and multy player.


Features and todos:

- [ ] Rendering system
  - [x] Basic features:
    - [x] Camera
    - [x] Texture atlas
    - [x] Render block faces instanced   
  - [ ] Advanced features:
	- [ ] 3D models
	- [x] Animations
	- [x] Transparency
	- [x] Sky box
  - [ ] Shaders:
	- [x] No visual artefacts on textures
	- [x] PBR pipeline
	- [x] Lights
	- [ ] Sky Box reflection
	- [ ] SSR
	- [ ] HBAO / SSAO / or even better SSDO
	- [x] HDR, ACES tonemapping
	- [ ] Bloom
	- [ ] Automatic exposure
	- [ ] Lens flare
	- [ ] Color grading
	- [x] Fog
	- [x] Underwater fog -(todo improve)
	- [ ] Sun Shadows (cascaded shadow maps)
	- [ ] God rays	
	- [x] Fake Shadows for all light types (todo improve)
	- [ ] Depth of field (maybe blur far stuff)
	- [ ] world curvature maybe?

- [x] Chunk system

- [ ] Multy player
  - [x] Connection to server and hand shake
  - [x] Server can validate moves
  - [ ] Server knows player position to optimize chunk logic stuff
