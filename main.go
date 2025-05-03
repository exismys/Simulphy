package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

func main() {
	rl.InitWindow(windowWidth, windowHeight, "Physics Simulations in Golang")
	defer rl.CloseWindow()
	rl.SetTargetFPS(targetFPS)

	// Initialize simulation
	sim := NewSimulation()
	sim.Run()
}
