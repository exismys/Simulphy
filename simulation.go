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
	setPosition(rl.Vector2)
	setTranslucent(bool)
}

type Simulation struct {
	objects      []SimObject
	ghostObject  SimObject
	buttons      []*Button
	inventory    Inventory
	cameraOffset rl.Vector2
}

func NewSimulation() *Simulation {
	sim := &Simulation{
		objects:      make([]SimObject, 0),
		buttons:      make([]*Button, 0),
		cameraOffset: rl.NewVector2(0, 0),
	}
	sim.inventory = *NewInventory(
		rl.NewVector2(20, 200),
		[]string{"AND", "NOT", "CIRCLE"},
		50,
		func(item string) {
			fmt.Println("-> Adding object: ", item)
			sim.setGhostObject(item)
			// sim.addObject(item)
		},
	)

	// Initialize button
	sim.buttons = append(sim.buttons, &Button{
		Pos:   rl.NewVector2(20, float32(simHeight)+float32(windowHeight-simHeight)/2-25),
		Size:  rl.NewVector2(100, 50),
		Label: "ADD",
		OnClick: func() {
			fmt.Println("The Add button was clicked!")
			sim.inventory.Visible = true
		},
	})

	return sim
}

func (sim *Simulation) Run() {
	dragStart := rl.NewVector2(0, 0)
	isDragging := false
	for !rl.WindowShouldClose() {
		if rl.IsMouseButtonPressed(rl.MouseRightButton) {
			isDragging = true
			dragStart = rl.GetMousePosition()
		}
		if isDragging {
			mouseDelta := rl.Vector2Subtract(dragStart, rl.GetMousePosition())
			sim.cameraOffset = rl.Vector2Add(sim.cameraOffset, mouseDelta)
			dragStart = rl.GetMousePosition()
		}
		if rl.IsMouseButtonReleased(rl.MouseButtonRight) {
			isDragging = false
		}

		if sim.ghostObject != nil && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
			sim.ghostObject.setPosition(rl.Vector2Add(rl.GetMousePosition(), sim.cameraOffset))
			sim.ghostObject.setTranslucent(false)
			sim.objects = append(sim.objects, sim.ghostObject)
			sim.ghostObject = nil
		}

		sim.HandleInput()
		// sim.Update()
		sim.Render()
	}
}

func (sim *Simulation) HandleInput() {
	for _, btn := range sim.buttons {
		btn.HandleInput()
	}
	sim.inventory.HandleInput()
}

func (sim *Simulation) Update() {
	for _, obj := range sim.objects {
		obj.update()
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
	sim.inventory.Draw()

	// Draw ghost object
	if sim.ghostObject != nil {
		sim.ghostObject.setPosition(rl.GetMousePosition())
		cameraOffset := rl.NewVector2(0, 0)
		sim.ghostObject.draw(&cameraOffset)
	}

	rl.EndDrawing()
}

func (sim *Simulation) setGhostObject(item string) {
	if item == "CIRCLE" {
		sim.ghostObject = &Circle{
			pos:    rl.GetMousePosition(),
			radius: 20,
			color:  rl.Color{R: 255, G: 0, B: 0, A: 128},
		}
	} else if item == "NOT" {
		sim.ghostObject = &NotGate{
			pos:   rl.GetMousePosition(),
			color: rl.Color{R: 128, G: 128, B: 128, A: 128},
		}
	}
}
