# ourCraft

It is the third time I have tried to make Minecraft from scratch.
This time I want to add a lot of harder-to-implement features like transparent blocks, light shadows, and multi-player!

Go check out the videos on [youtube about it](https://www.youtube.com/watch?v=StNAG_tLEoU&list=PLKUl_fMWLdH-0H-tz0S144g5xXliHOIxC&index=4)!

![image](https://github.com/meemknight/ourCraft/assets/36445656/3f6c8976-8f63-4259-a1de-3305c4c52467)


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
	- [x] Skybox
  - [ ] Shaders:
	- [x] No visual artifacts on textures
	- [x] PBR pipeline
	- [x] Lights
	- [ ] Sky Box reflection
	- [ ] SSR
	- [ ] HBAO / SSAO / or even better SSDO
	- [x] HDR, ACES tone mapping
	- [ ] Bloom
	- [ ] Automatic exposure
	- [ ] Lens flare
	- [ ] Color grading
	- [ ] Fog
	- [ ] Underwater fog -(todo improve)
	- [ ] Sun Shadows (cascaded shadow maps)
	- [ ] God rays	
	- [x] Fake Shadows for all light types (todo improve)
  	- [ ] Shadiows
	- [ ] Depth of field (maybe blur far stuff)
	- [ ] world curvature maybe?

- [x] Chunk system

- [ ] Multy player
  - [x] Connection to server and handshake
  - [x] Server can validate moves
  - [x] Server knows player position to optimize chunk logic stuff
  - [x] Undo Stuff On client
  - [ ] Buffering
  - [ ] Rubber banding
  - [ ] Entities
