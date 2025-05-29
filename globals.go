package main

// Simulation configs
const (
	simWidth       int32   = 1200
	simHeight      int32   = 720
	windowWidth    int32   = simWidth
	windowHeight   int32   = simHeight + 80
	targetFPS      int32   = 60
	fixedDeltaTime float32 = 1 / float32(targetFPS)
	dampingFactor  float32 = 0.9
)

// Global objects
var (
	wires    []*Wire
	ports    []*Port
	leds     []*Led
	notGates []*NotGate
	orGates  []*OrGate
	andGates []*AndGate
	circles  []*Circle
	powers   []*Power

	// This represents the "last clicked" input port of LED
	finalPort *Port
)
