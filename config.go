package main

const (
	simWidth       int32   = 1600
	simHeight      int32   = 900
	windowWidth    int32   = simWidth
	windowHeight   int32   = simHeight + 80
	targetFPS      int32   = 60
	fixedDeltaTime float32 = 1 / float32(targetFPS)
	dampingFactor  float32 = 0.9 // Todo: Remove this and implement proper collision with energy loss
)

// var colors [5]rl.Color = [...]rl.Color{rl.Yellow, rl.Pink, rl.Red, rl.Beige, rl.SkyBlue}
