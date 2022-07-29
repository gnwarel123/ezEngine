import ez = require("TypeScript/ez")

//export class TS_Old extends ez.TypescriptComponent {
export class TS_Old extends ez.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    TestColor: ez.Color = new ez.Color(1, 0, 0, 1);
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    static RegisterMessageHandlers() {

        // you can only call "RegisterMessageHandler" from within this function
        ez.TypescriptComponent.RegisterMessageHandler(ez.MsgSetColor, "OnMsgSetColor");
    }

    Initialize(): void {
        ez.Log.Info(this.GetOwner().GetName() + ": Initialized called. My color is: " + JSON.stringify(this.TestColor));
    }

    Deinitialize(): void {
        ez.Log.Info(this.GetOwner().GetName() + ": Deinitialized called");
    }

    OnActivated(): void {
        ez.Log.Info(this.GetOwner().GetName() + ": OnActivated called");
    }

    OnDeactivated(): void {
        ez.Log.Info(this.GetOwner().GetName() + ": OnDeactivated called");
    }

    OnSimulationStarted(): void {
        ez.Log.Info(this.GetOwner().GetName() + ": OnSimulationStarted called");
    }

    OnMsgSetColor(msg: ez.MsgSetColor): void {
        this.TestColor = msg.Color;
    }

    Tick(): void {
        // if a regular tick is not needed, remove this and derive directly from ez.TypescriptComponent
        ez.Debug.Draw3DText(this.GetOwner().GetName(), this.GetOwner().GetGlobalPosition(), this.TestColor);
    }
}

