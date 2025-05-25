package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type AndGate struct {
	pos        rl.Vector2
	color      rl.Color
	inputPortA *Port
	inputPortB *Port
	outputPort *Port
}

func NewAndGate(sim *Simulation, position rl.Vector2, color rl.Color) *AndGate {
	ag := &AndGate{
		pos:   position,
		color: color,
	}
	ag.inputPortA = &Port{
		pos:       rl.NewVector2(ag.pos.X-26, ag.pos.Y-10),
		radius:    5,
		color:     rl.SkyBlue,
		inputPort: true,
		fromPorts: []*Port{},
	}
	ag.inputPortB = &Port{
		pos:       rl.NewVector2(ag.pos.X-26, ag.pos.Y+10),
		radius:    5,
		color:     rl.SkyBlue,
		inputPort: true,
		fromPorts: []*Port{},
	}
	ag.outputPort = &Port{
		pos:        rl.NewVector2(ag.pos.X+26, ag.pos.Y),
		radius:     5,
		color:      rl.Orange,
		inputPorts: []*Port{ag.inputPortA, ag.inputPortB},
		resMethod:  AND,
	}
	ag.inputPortA.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.ghostObject != nil {
			w := sim.ghostObject.(*Wire)
			w.To = ag.inputPortA.pos
			w.ToPort = ag.inputPortA
			w.From = rl.Vector2Add(w.From, sim.cameraOffset)
			wires = append(wires, w)
			ag.inputPortA.fromPorts = append(ag.inputPortA.fromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.objects = append(sim.objects, w)
			sim.ghostObject = nil
		}
	}
	ag.inputPortB.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.ghostObject != nil {
			w := sim.ghostObject.(*Wire)
			w.To = ag.inputPortB.pos
			w.ToPort = ag.inputPortB
			w.From = rl.Vector2Add(w.From, sim.cameraOffset)
			wires = append(wires, w)
			ag.inputPortB.fromPorts = append(ag.inputPortB.fromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.objects = append(sim.objects, w)
			sim.ghostObject = nil
		}
	}
	ag.outputPort.onClick = func() {
		fmt.Println("Output port of NOT gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(ag.outputPort.pos, sim.cameraOffset),
			FromPort: ag.outputPort,
		}
		sim.ghostObject = &wire
	}
	ag.inputPortA.color.A = 128
	ag.inputPortB.color.A = 128
	ag.outputPort.color.A = 128
	return ag
}

func (ag *AndGate) draw(cameraOffset *rl.Vector2) {
	pos := rl.Vector2Subtract(ag.pos, *cameraOffset)

	rl.DrawRectangleRec(
		rl.NewRectangle(pos.X-20, pos.Y-20, 20, 40),
		ag.color,
	)
	rl.DrawCircleSector(pos, 20, -90, 90, 32, ag.color)

	ag.inputPortA.draw(cameraOffset)
	ag.inputPortB.draw(cameraOffset)
	ag.outputPort.draw(cameraOffset)
}

func (ag *AndGate) update() {
}

func (ag *AndGate) isDynamic() bool {
	return false
}

func (ag *AndGate) setPosition(position rl.Vector2) {
	ag.pos = position
	ag.inputPortA.pos = rl.NewVector2(position.X-26, position.Y-10)
	ag.inputPortB.pos = rl.NewVector2(position.X-26, position.Y+10)
	ag.outputPort.pos = rl.NewVector2(position.X+26, position.Y)
}

func (ag *AndGate) setTranslucent(set bool) {
	if set {
		ag.color.A = 128
		ag.inputPortA.color.A = 128
		ag.inputPortB.color.A = 128
		ag.outputPort.color.A = 128
	} else {
		ag.color.A = 255
		ag.inputPortA.color.A = 255
		ag.inputPortB.color.A = 255
		ag.outputPort.color.A = 255
	}
}

func (ag *AndGate) HandleInput() {
	ag.inputPortA.HandleInput()
	ag.inputPortB.HandleInput()
	ag.outputPort.HandleInput()
}
