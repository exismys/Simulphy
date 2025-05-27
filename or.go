package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type OrGate struct {
	Pos        rl.Vector2
	Color      rl.Color
	InputPortA *Port
	InputPortB *Port
	outputPort *Port
}

func NewOrGate(sim *Simulation, Position rl.Vector2, Color rl.Color) *OrGate {
	og := &OrGate{
		Pos:   Position,
		Color: Color,
	}
	og.InputPortA = &Port{
		Pos:         rl.NewVector2(og.Pos.X-26, og.Pos.Y-10),
		Radius:      5,
		Color:       rl.SkyBlue,
		IsInputPort: true,
		FromPorts:   []*Port{},
	}
	og.InputPortB = &Port{
		Pos:         rl.NewVector2(og.Pos.X-26, og.Pos.Y+10),
		Radius:      5,
		Color:       rl.SkyBlue,
		IsInputPort: true,
		FromPorts:   []*Port{},
	}
	og.outputPort = &Port{
		Pos:        rl.NewVector2(og.Pos.X+26, og.Pos.Y),
		Radius:     5,
		Color:      rl.Orange,
		InputPorts: []*Port{og.InputPortA, og.InputPortB},
		ResMethod:  OR,
	}
	og.InputPortA.onClick = func() {
		fmt.Println("Input port of OR gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = og.InputPortA.Pos
			w.ToPort = og.InputPortA
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			og.InputPortA.FromPorts = append(og.InputPortA.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	og.InputPortB.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = og.InputPortB.Pos
			w.ToPort = og.InputPortB
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			og.InputPortB.FromPorts = append(og.InputPortB.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	og.outputPort.onClick = func() {
		fmt.Println("Output port of OR gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(og.outputPort.Pos, sim.CameraOffset),
			FromPort: og.outputPort,
		}
		sim.GhostObject = &wire
	}
	og.InputPortA.Color.A = 128
	og.InputPortB.Color.A = 128
	og.outputPort.Color.A = 128
	return og
}

func (og *OrGate) draw(CameraOffset *rl.Vector2) {
	Pos := rl.Vector2Subtract(og.Pos, *CameraOffset)

	rl.DrawRectangleRec(
		rl.NewRectangle(Pos.X-20, Pos.Y-20, 20, 40),
		og.Color,
	)
	cutColor := rl.NewColor(0, 0, 0, og.Color.A)
	rl.DrawCircleSector(rl.NewVector2(Pos.X-20, Pos.Y), 20, -90, 90, 32, cutColor)
	rl.DrawCircleSector(rl.NewVector2(Pos.X, Pos.Y), 20, -90, 90, 32, og.Color)

	og.InputPortA.draw(CameraOffset)
	og.InputPortB.draw(CameraOffset)
	og.outputPort.draw(CameraOffset)
}

func (og *OrGate) update() {
}

func (og *OrGate) isDynamic() bool {
	return false
}

func (og *OrGate) setPosition(Position rl.Vector2) {
	og.Pos = Position
	og.InputPortA.Pos = rl.NewVector2(Position.X-26, Position.Y-10)
	og.InputPortB.Pos = rl.NewVector2(Position.X-26, Position.Y+10)
	og.outputPort.Pos = rl.NewVector2(Position.X+26, Position.Y)
}

func (og *OrGate) setTranslucent(set bool) {
	if set {
		og.Color.A = 128
		og.InputPortA.Color.A = 128
		og.InputPortB.Color.A = 128
		og.outputPort.Color.A = 128
	} else {
		og.Color.A = 255
		og.InputPortA.Color.A = 255
		og.InputPortB.Color.A = 255
		og.outputPort.Color.A = 255
	}
}

func (og *OrGate) HandleInput() {
	og.InputPortA.HandleInput()
	og.InputPortB.HandleInput()
	og.outputPort.HandleInput()
}
