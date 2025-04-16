package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type SimObject interface {
	update()
	draw(cameraOffset *rl.Vector2)
	isDynamic() bool
	isClicked() bool
}

type Simulation struct {
	objects      []SimObject
	buttons      []*Button
	inventory    []*Inventory
	cameraOffset rl.Vector2
}

func NewSimulation() *Simulation {
	sim := &Simulation{
		objects:      make([]SimObject, 0),
		buttons:      make([]*Button, 0),
		cameraOffset: rl.NewVector2(0, 0),
	}

	// Initialize button
	sim.buttons = append(sim.buttons, &Button{
		Pos:   rl.NewVector2(20, float32(simHeight)+float32(windowHeight-simHeight)/2-25),
		Size:  rl.NewVector2(100, 50),
		Label: "Add",
		OnClick: func() {
			fmt.Println("The button was clicked!")
		},
	})

	return sim
}

func (sim *Simulation) Run() {
	dragStart := rl.NewVector2(0, 0)
	isDragging := false
	for !rl.WindowShouldClose() {
		if rl.IsMouseButtonPressed(rl.MouseLeftButton) {
			isDragging = true
			dragStart = rl.GetMousePosition()
		}
		if isDragging {
			mouseDelta := rl.Vector2Subtract(dragStart, rl.GetMousePosition())
			sim.cameraOffset = rl.Vector2Add(sim.cameraOffset, mouseDelta)
			dragStart = rl.GetMousePosition()
		}
		if rl.IsMouseButtonReleased(rl.MouseButtonLeft) {
			isDragging = false
		}

		sim.HandleInput()
		sim.Render()
	}
}

func (sim *Simulation) HandleInput() {
	for _, btn := range sim.buttons {
		btn.HandleInput()
	}

}

func (sim *Simulation) Render() {
	rl.BeginDrawing()
	rl.ClearBackground(rl.Black)

	// Draw objects
	for _, c := range sim.objects {
		c.draw(&sim.cameraOffset)
	}

	// Draw UI
	rl.DrawLine(0, simHeight, windowWidth, simHeight, rl.Gray)
	for _, btn := range sim.buttons {
		btn.Draw()
	}

	rl.EndDrawing()
}
