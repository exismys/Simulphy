package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type OrGate struct {
	Pos        rl.Vector2
	Color      rl.Color
	inputPortA *Port
	inputPortB *Port
	outputPort *Port
}

func NewOrGate(sim *Simulation, Position rl.Vector2, Color rl.Color) *OrGate {
	og := &OrGate{
		Pos:   Position,
		Color: Color,
	}
	og.inputPortA = &Port{
		Pos:         rl.NewVector2(og.Pos.X-26, og.Pos.Y-10),
		Radius:      5,
		Color:       rl.SkyBlue,
		IsInputPort: true,
		FromPorts:   []*Port{},
	}
	og.inputPortB = &Port{
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
		InputPorts: []*Port{og.inputPortA, og.inputPortB},
		ResMethod:  OR,
	}
	og.inputPortA.OnClick = func() {
		fmt.Println("Input port of OR gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = og.inputPortA.Pos
			w.ToPort = og.inputPortA
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			og.inputPortA.FromPorts = append(og.inputPortA.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	og.inputPortB.OnClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = og.inputPortB.Pos
			w.ToPort = og.inputPortB
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			og.inputPortB.FromPorts = append(og.inputPortB.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	og.outputPort.OnClick = func() {
		fmt.Println("Output port of OR gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(og.outputPort.Pos, sim.CameraOffset),
			FromPort: og.outputPort,
		}
		sim.GhostObject = &wire
	}
	og.inputPortA.Color.A = 128
	og.inputPortB.Color.A = 128
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

	og.inputPortA.draw(CameraOffset)
	og.inputPortB.draw(CameraOffset)
	og.outputPort.draw(CameraOffset)
}

func (og *OrGate) update() {
}

func (og *OrGate) isDynamic() bool {
	return false
}

func (og *OrGate) setPosition(Position rl.Vector2) {
	og.Pos = Position
	og.inputPortA.Pos = rl.NewVector2(Position.X-26, Position.Y-10)
	og.inputPortB.Pos = rl.NewVector2(Position.X-26, Position.Y+10)
	og.outputPort.Pos = rl.NewVector2(Position.X+26, Position.Y)
}

func (og *OrGate) setTranslucent(set bool) {
	if set {
		og.Color.A = 128
		og.inputPortA.Color.A = 128
		og.inputPortB.Color.A = 128
		og.outputPort.Color.A = 128
	} else {
		og.Color.A = 255
		og.inputPortA.Color.A = 255
		og.inputPortB.Color.A = 255
		og.outputPort.Color.A = 255
	}
}

func (og *OrGate) HandleInput() {
	og.inputPortA.HandleInput()
	og.inputPortB.HandleInput()
	og.outputPort.HandleInput()
}
