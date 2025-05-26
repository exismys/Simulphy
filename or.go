package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type OrGate struct {
	pos        rl.Vector2
	color      rl.Color
	inputPortA *Port
	inputPortB *Port
	outputPort *Port
}

func NewOrGate(sim *Simulation, position rl.Vector2, color rl.Color) *OrGate {
	og := &OrGate{
		pos:   position,
		color: color,
	}
	og.inputPortA = &Port{
		pos:       rl.NewVector2(og.pos.X-26, og.pos.Y-10),
		radius:    5,
		color:     rl.SkyBlue,
		inputPort: true,
		fromPorts: []*Port{},
	}
	og.inputPortB = &Port{
		pos:       rl.NewVector2(og.pos.X-26, og.pos.Y+10),
		radius:    5,
		color:     rl.SkyBlue,
		inputPort: true,
		fromPorts: []*Port{},
	}
	og.outputPort = &Port{
		pos:        rl.NewVector2(og.pos.X+26, og.pos.Y),
		radius:     5,
		color:      rl.Orange,
		inputPorts: []*Port{og.inputPortA, og.inputPortB},
		resMethod:  OR,
	}
	og.inputPortA.onClick = func() {
		fmt.Println("Input port of OR gate clicked!")
		if sim.ghostObject != nil {
			w := sim.ghostObject.(*Wire)
			w.To = og.inputPortA.pos
			w.ToPort = og.inputPortA
			w.From = rl.Vector2Add(w.From, sim.cameraOffset)
			wires = append(wires, w)
			og.inputPortA.fromPorts = append(og.inputPortA.fromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.objects = append(sim.objects, w)
			sim.ghostObject = nil
		}
	}
	og.inputPortB.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.ghostObject != nil {
			w := sim.ghostObject.(*Wire)
			w.To = og.inputPortB.pos
			w.ToPort = og.inputPortB
			w.From = rl.Vector2Add(w.From, sim.cameraOffset)
			wires = append(wires, w)
			og.inputPortB.fromPorts = append(og.inputPortB.fromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.objects = append(sim.objects, w)
			sim.ghostObject = nil
		}
	}
	og.outputPort.onClick = func() {
		fmt.Println("Output port of OR gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(og.outputPort.pos, sim.cameraOffset),
			FromPort: og.outputPort,
		}
		sim.ghostObject = &wire
	}
	og.inputPortA.color.A = 128
	og.inputPortB.color.A = 128
	og.outputPort.color.A = 128
	return og
}

func (og *OrGate) draw(cameraOffset *rl.Vector2) {
	pos := rl.Vector2Subtract(og.pos, *cameraOffset)

	rl.DrawRectangleRec(
		rl.NewRectangle(pos.X-20, pos.Y-20, 20, 40),
		og.color,
	)
	rl.DrawCircleSector(rl.NewVector2(pos.X-20, pos.Y), 20, -90, 90, 32, rl.Black)
	rl.DrawCircleSector(rl.NewVector2(pos.X, pos.Y), 20, -90, 90, 32, og.color)

	og.inputPortA.draw(cameraOffset)
	og.inputPortB.draw(cameraOffset)
	og.outputPort.draw(cameraOffset)
}

func (og *OrGate) update() {
}

func (og *OrGate) isDynamic() bool {
	return false
}

func (og *OrGate) setPosition(position rl.Vector2) {
	og.pos = position
	og.inputPortA.pos = rl.NewVector2(position.X-26, position.Y-10)
	og.inputPortB.pos = rl.NewVector2(position.X-26, position.Y+10)
	og.outputPort.pos = rl.NewVector2(position.X+26, position.Y)
}

func (og *OrGate) setTranslucent(set bool) {
	if set {
		og.color.A = 128
		og.inputPortA.color.A = 128
		og.inputPortB.color.A = 128
		og.outputPort.color.A = 128
	} else {
		og.color.A = 255
		og.inputPortA.color.A = 255
		og.inputPortB.color.A = 255
		og.outputPort.color.A = 255
	}
}

func (og *OrGate) HandleInput() {
	og.inputPortA.HandleInput()
	og.inputPortB.HandleInput()
	og.outputPort.HandleInput()
}
