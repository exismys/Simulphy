package main

const (
	simWidth       int32   = 1200
	simHeight      int32   = 720
	windowWidth    int32   = simWidth
	windowHeight   int32   = simHeight + 80
	targetFPS      int32   = 60
	fixedDeltaTime float32 = 1 / float32(targetFPS)
	dampingFactor  float32 = 0.9
)
