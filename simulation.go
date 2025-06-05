package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type SimObject interface {
	update()
	draw(cameraOffset *rl.Vector2)
	isDynamic() bool
	setPosition(rl.Vector2)
	setTranslucent(bool)
	HandleInput()
}

type Simulation struct {
	Objects      []SimObject
	GhostObject  SimObject
	Buttons      []*Button
	Inventory    Inventory
	SimStateInv  Inventory
	CameraOffset rl.Vector2
}

func NewSimulation() *Simulation {
	sim := &Simulation{
		Objects:      make([]SimObject, 0),
		Buttons:      make([]*Button, 0),
		CameraOffset: rl.NewVector2(0, 0),
	}

	// Initialize inventory
	sim.Inventory = *NewInventory(
		rl.NewVector2(20, float32(simHeight)-20),
		[]string{"AND", "CIRCLE", "LED", "NOT", "OR", "POWER"},
		50,
		100,
		func(item string) {
			fmt.Println("-> Adding object: ", item)
			sim.setGhostObject(item)
		},
	)

	// Initialize ADD button
	addBtn := &Button{
		Pos:   rl.NewVector2(20, float32(simHeight)+float32(windowHeight-simHeight)/2-25),
		Size:  rl.NewVector2(100, 50),
		Label: "ADD",
	}
	addBtn.onClick = func() {
		fmt.Println("The ADD button was clicked!")
		sim.Inventory.Visible = !sim.Inventory.Visible
		if addBtn.Label == "ADD" {
			addBtn.Label = "X"
		} else {
			addBtn.Label = "ADD"
		}
	}
	sim.Buttons = append(sim.Buttons, addBtn)

	// Initialize LOAD Button
	loadBtn := &Button{
		Pos:   rl.NewVector2(20+100+20, float32(simHeight)+float32(windowHeight-simHeight)/2-25),
		Size:  rl.NewVector2(100, 50),
		Label: "LOAD",
	}
	loadBtn.onClick = func() {
		fmt.Println("The LOAD button was clicked!")
		if loadBtn.Label == "LOAD" {
			stateFiles, err := getSimStateFiles()
			if err != nil {
				fmt.Println(err)
				return
			}
			sim.SimStateInv = *NewInventory(
				rl.NewVector2(20+100+20, float32(simHeight)-20),
				stateFiles,
				50,
				100,
				func(item string) {
					sim.SimStateInv.Visible = false
					loadBtn.Label = "LOAD"
					deserialize(sim, item)
				},
			)
			loadBtn.Label = "X"
		} else {
			loadBtn.Label = "LOAD"
		}
		sim.SimStateInv.Visible = !sim.SimStateInv.Visible
	}
	sim.Buttons = append(sim.Buttons, loadBtn)

	// Initialize SAVE Button
	saveBtn := &Button{
		Pos:   rl.NewVector2(20+100+20+100+20, float32(simHeight)+float32(windowHeight-simHeight)/2-25),
		Size:  rl.NewVector2(100, 50),
		Label: "SAVE",
	}
	saveBtn.onClick = func() {
		fmt.Println("The SAVE button was clicked!")
		// To Do: Create a prompt UI to prompt for filename for saving
		// prompt()
		serialize("circuit")
	}
	sim.Buttons = append(sim.Buttons, saveBtn)

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
			sim.CameraOffset = rl.Vector2Add(sim.CameraOffset, mouseDelta)
			dragStart = rl.GetMousePosition()
		}
		if rl.IsMouseButtonReleased(rl.MouseButtonRight) {
			isDragging = false
		}

		if sim.GhostObject != nil && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
			if _, ok := sim.GhostObject.(*Wire); !ok {
				// The cameraOffset is added to the mousePosition (the original position) in order to
				// neutralize the draw method which draws at pos - cameraOffset
				sim.GhostObject.setPosition(rl.Vector2Add(rl.GetMousePosition(), sim.CameraOffset))
				sim.GhostObject.setTranslucent(false)
				sim.Objects = append(sim.Objects, sim.GhostObject)
			}

			// Append objects into their own concrete type array
			if ng, ok := sim.GhostObject.(*NotGate); ok {
				notGates = append(notGates, ng)
			} else if og, ok := sim.GhostObject.(*OrGate); ok {
				orGates = append(orGates, og)
			} else if ag, ok := sim.GhostObject.(*AndGate); ok {
				andGates = append(andGates, ag)
			} else if l, ok := sim.GhostObject.(*Led); ok {
				leds = append(leds, l)
			} else if p, ok := sim.GhostObject.(*Power); ok {
				powers = append(powers, p)
			}

			if _, ok := sim.GhostObject.(*Wire); !ok {
				sim.GhostObject = nil
			}
		}

		sim.HandleInput()
		// sim.Update()
		sim.Render()
	}
}

func (sim *Simulation) HandleInput() {
	for _, btn := range sim.Buttons {
		btn.HandleInput()
	}
	sim.Inventory.HandleInput()
	sim.SimStateInv.HandleInput()
	for _, obj := range sim.Objects {
		obj.HandleInput()
	}
}

func (sim *Simulation) Update() {
	for _, obj := range sim.Objects {
		obj.update()
	}
}

func (sim *Simulation) Render() {
	rl.BeginDrawing()
	rl.ClearBackground(rl.Black)

	// Draw objects
	for _, obj := range sim.Objects {
		obj.draw(&sim.CameraOffset)
	}

	// Draw UI
	rl.DrawLine(0, simHeight, windowWidth, simHeight, rl.Gray)
	for _, btn := range sim.Buttons {
		btn.Draw()
	}
	sim.Inventory.Draw()
	sim.SimStateInv.Draw()

	// Draw ghost object
	if sim.GhostObject != nil {
		sim.GhostObject.setPosition(rl.GetMousePosition())
		cameraOffset := rl.NewVector2(0, 0)
		sim.GhostObject.draw(&cameraOffset)
	}

	rl.EndDrawing()
}

func (sim *Simulation) setGhostObject(item string) {
	if item == "CIRCLE" {
		sim.GhostObject = &Circle{
			Pos:    rl.GetMousePosition(),
			Radius: 20,
			Color:  rl.Color{R: 255, G: 0, B: 0, A: 128},
		}
	} else if item == "NOT" {
		pos := rl.GetMousePosition()
		color := rl.Color{R: 128, G: 128, B: 128, A: 128}
		sim.GhostObject = NewNotGate(sim, pos, color)
	} else if item == "AND" {
		pos := rl.GetMousePosition()
		color := rl.Color{R: 128, G: 128, B: 128, A: 128}
		sim.GhostObject = NewAndGate(sim, pos, color)
	} else if item == "OR" {
		pos := rl.GetMousePosition()
		color := rl.Color{R: 128, G: 128, B: 128, A: 128}
		sim.GhostObject = NewOrGate(sim, pos, color)
	} else if item == "POWER" {
		pos := rl.GetMousePosition()
		sim.GhostObject = NewPowerSource(sim, pos)
		sim.GhostObject.setTranslucent(true)
	} else if item == "LED" {
		pos := rl.GetMousePosition()
		sim.GhostObject = NewLed(sim, pos)
		sim.GhostObject.setTranslucent(true)
	}
}
