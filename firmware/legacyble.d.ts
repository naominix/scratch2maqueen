declare namespace LegacyBle {
    //% shim=LegacyBle::start
    function start(): number;
    //% shim=LegacyBle::command
    function command(slot: number): number;
    //% shim=LegacyBle::publish
    function publish(frame: Buffer): void;
}
