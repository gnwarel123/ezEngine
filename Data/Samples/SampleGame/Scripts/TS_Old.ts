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

    // Initialize(): void { }
    // Deinitialize(): void { }
    // OnActivated(): void { }
    // OnDeactivated(): void { }

    OnSimulationStarted(): void {
        //this.SetTickInterval(ez.Time.Milliseconds(100));
    }

    OnMsgSetColor(msg: ez.MsgSetColor): void {
        this.TestColor = msg.Color;
    }

    Tick(): void {
        // if a regular tick is not needed, remove this and derive directly from ez.TypescriptComponent
        ez.Debug.Draw3DText(this.GetOwner().GetName(), this.GetOwner().GetGlobalPosition(), this.TestColor);
    }
}

