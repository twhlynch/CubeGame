# Planned features

### Object properties

Objects should have differing properties based on color. This is like mass, friction, restitution, gravity, etc.

### Discoverable achievements

Would be a hidden list of random things that you can do, such as stack 10 objects, throw some distance, juggle, etc.

### Extra tools

Some kind of tools menu accessable by holding pinch in an ok guesture that pops up after a second and you can move sideways or up and down to choose an action.

- separating objects
- editing object properties

### Toggle debug tools

Should be added in the main menu

- hand indicators
- UI projection indicator

### Space Setup

Use metas space setup & scene data apis for getting tables, walls, and so on, and give them collision.
Has two parts to it. Live detecting surroundings, and manually chosen objects.

Requires implementing support for these in Rayne.

- https://developers.meta.com/horizon/documentation/native/android/openxr-scene-overview/
- https://developers.meta.com/horizon/documentation/native/android/mobile-scene-api-ref/

### Passthrough Depth Occlusion

When a cube is partially behind your hand or a table or wall, etc, it should be partially or fully occluded.

Requires implementing support for the Depth API in Rayne.

- https://developers.meta.com/horizon/documentation/unity/unity-depthapi-occlusions/
- https://registry.khronos.org/OpenXR/specs/1.1/man/html/XrEnvironmentDepthImageAcquireInfoMETA.html
- https://registry.khronos.org/OpenXR/specs/1.1/man/html/XrEnvironmentDepthImageMETA.html

### More stable holding

- Adjust pinching threshhold with size of hand. Might need a way to calibrate depending on whether constant updating works any good.
- Prevent dropping if your hand is covered by your other hand.
- Preserve hands state when behind you / out of view.

### Accurate hand hitboxes

Hitbox on hands so you can punch stuff or carry something more realistically. Ideally in a way that works well with the pinching mechanic.

### Statistics menu

A stats menu that shows how many things youve created, or thrown. Would require saving data to headset.

### Controller support

Maybe, though its a little besides the point of the game.
