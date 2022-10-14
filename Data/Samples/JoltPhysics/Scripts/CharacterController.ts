import ez = require("TypeScript/ez")

//export class CharacterController extends ez.TypescriptComponent {
export class CharacterController extends ez.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    /* END AUTO-GENERATED: VARIABLES */

    characterController: ez.JoltDefaultCharacterComponent = null;
    input: ez.InputComponent = null;
    camera: ez.GameObject = null;
    headBone: ez.HeadBoneComponent = null;
    grabObject: ez.JoltGrabObjectComponent = null;
    flashlightObj: ez.GameObject = null;
    flashlight: ez.SpotLightComponent = null;

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        ez.TypescriptComponent.RegisterMessageHandler(ez.MsgInputActionTriggered, "OnMsgInputActionTriggered");
    }

    OnSimulationStarted(): void {
        this.SetTickInterval(ez.Time.Milliseconds(0));

        let owner = this.GetOwner();
        this.characterController = owner.TryGetComponentOfBaseType(ez.JoltDefaultCharacterComponent);
        this.input = owner.TryGetComponentOfBaseType(ez.InputComponent);
        this.camera = owner.FindChildByName("Camera", true);
        this.headBone = this.camera.TryGetComponentOfBaseType(ez.HeadBoneComponent);
        this.grabObject = owner.FindChildByName("GrabObject", true).TryGetComponentOfBaseType(ez.JoltGrabObjectComponent);
        this.flashlightObj = owner.FindChildByName("Flashlight", true);
        this.flashlight = this.flashlightObj.TryGetComponentOfBaseType(ez.SpotLightComponent);
    }

    Tick(): void {

        // character controller update
        {
            let msg = new ez.MsgMoveCharacterController();

            msg.Jump = this.input.GetCurrentInputState("Jump", true) > 0.5;
            msg.MoveForwards = this.input.GetCurrentInputState("MoveForwards", false);
            msg.MoveBackwards = this.input.GetCurrentInputState("MoveBackwards", false);
            msg.StrafeLeft = this.input.GetCurrentInputState("StrafeLeft", false);
            msg.StrafeRight = this.input.GetCurrentInputState("StrafeRight", false);
            msg.RotateLeft = this.input.GetCurrentInputState("RotateLeft", false);
            msg.RotateRight = this.input.GetCurrentInputState("RotateRight", false);
            msg.Run = this.input.GetCurrentInputState("Run", false) > 0.5;
            msg.Crouch = this.input.GetCurrentInputState("Crouch", false) > 0.5;

            this.characterController.SendMessage(msg);
        }

        // look up / down
        {
            let up = this.input.GetCurrentInputState("LookUp", false);
            let down = this.input.GetCurrentInputState("LookDown", false);

            this.headBone.ChangeVerticalRotation(down - up);
        }
    }

    OnMsgInputActionTriggered(msg: ez.MsgInputActionTriggered): void {

        if (msg.TriggerState == ez.TriggerState.Activated) {

            if (msg.InputAction == "Flashlight") {
                this.flashlight.SetActiveFlag(!this.flashlight.GetActiveFlag());
            }

            if (msg.InputAction == "Use") {

                if (this.grabObject.HasObjectGrabbed()) {
                    this.grabObject.DropGrabbedObject();
                }
                else if (this.grabObject.GrabNearbyObject()) {
                }
            }

            if (msg.InputAction == "Shoot") {

                if (this.grabObject.HasObjectGrabbed()) {
                    let dir = new ez.Vec3(20.75, 0, 0);
                    this.grabObject.ThrowGrabbedObject(dir);
                }
            }

            if (msg.InputAction == "Teleport") {
                // let owner = this.characterController.GetOwner();
                // let pos = owner.GetGlobalPosition();
                // let dir = owner.GetGlobalDirForwards();
                // dir.z = 0;
                // dir.Normalize();
                // dir.MulNumber(5.0);
                // pos.AddVec3(dir);

                // if (this.characterController.IsDestinationUnobstructed(pos, 0)) {
                //     this.characterController.TeleportCharacter(pos);
                // }
            }
        }
    }
}

