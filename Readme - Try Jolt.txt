How to try EZ with Jolt.

1. Run GenerateWin64vs2022.bat
2. Open "Workspace\vs2022x64\ezEngine2_vs2022x64.sln"
3. Compile everything (I always build in Debug, performance is still more than good enough)
   Note that Jolt is compiled straight from source into EZ, so you can step through it.
4. Launch "Editor"
5. From the Dashboard under "Samples" open the "JoltPhysics" project.
6. If asked to transform assets, select "Apply".
7. Open scene "Playground".

To simulate the scene (without playing yourself) select the "Play" button in the toolbar, or press F5.
To play the scene with the character controller, select the "Controller" button in the toolbar or press CTRL+F5.
  A separate window will open. 
	WSAD+Mouse, Space to jump, Shift to run, Ctrl to crouch
	ESC to quit
	F1 to open the console (https://ezengine.net/pages/docs/debugging/console.html)
		TAB to list commands and auto-complete.
		Page Up/Down to scroll.
		For instance, enter "App.VSync =" to toggle the state of VSync.
	F2 to repeat the last command (makes toggling back and forth easy).
	
Note that stair stepping doesn't work at high framerates.
CVars (e.g. 'App.VSync') can also be modified in the editor CVar panel.

I now do a manual "foot check" where I use a small capsule at the ground to detect whether there are any contacts pointing up.
I then only allow stair stepping, jumping, stick to ground when there is such a contact.
This also allows to enable pushing the CC down (sliding) when Jolt says it is OnGround, but it only touches something at the side.
This then prevents stepping over steep walls.

* With the CVar "Jolt.StickToGround" the stick-to-ground feature can be disabled. Makes quite the difference!* 
* With the CVar "Jolt.CC.FootCheck" the foot check can be disabled. Gives you more of the default Jolt CC behavior.


Known Issues:

1. Sometimes Jolt asserts that it didn't find the expected contact.
2. Stair-stepping doesn't always work in general.
3. Stair-stepping rarely works at high framerates (150+).
4. With foot-check disabled, one can step over the steep wall.


What code to look at:

ezJoltDefaultCharacterComponent::UpdateCharacter() in JoltDefaultCharacterComponent.cpp is the most interesting part.
The stair stepping is executed in ezJoltCharacterControllerComponent::RawMoveWithVelocity() in JoltCharacterControllerComponent.cpp.
