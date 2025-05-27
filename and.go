package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type AndGate struct {
	Pos        rl.Vector2
	Color      rl.Color
	InputPortA *Port
	InputPortB *Port
	OutputPort *Port
}

func NewAndGate(sim *Simulation, Position rl.Vector2, Color rl.Color) *AndGate {
	ag := &AndGate{
		Pos:   Position,
		Color: Color,
	}
	ag.InputPortA = &Port{
		Pos:         rl.NewVector2(ag.Pos.X-26, ag.Pos.Y-10),
		Radius:      5,
		Color:       rl.SkyBlue,
		IsInputPort: true,
		FromPorts:   []*Port{},
	}
	ag.InputPortB = &Port{
		Pos:         rl.NewVector2(ag.Pos.X-26, ag.Pos.Y+10),
		Radius:      5,
		Color:       rl.SkyBlue,
		IsInputPort: true,
		FromPorts:   []*Port{},
	}
	ag.OutputPort = &Port{
		Pos:        rl.NewVector2(ag.Pos.X+26, ag.Pos.Y),
		Radius:     5,
		Color:      rl.Orange,
		InputPorts: []*Port{ag.InputPortA, ag.InputPortB},
		ResMethod:  AND,
	}
	ag.InputPortA.OnClick = func() {
		fmt.Println("Input port of AND gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = ag.InputPortA.Pos
			w.ToPort = ag.InputPortA
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			ag.InputPortA.FromPorts = append(ag.InputPortA.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	ag.InputPortB.OnClick = func() {
		fmt.Println("Input port of AND gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = ag.InputPortB.Pos
			w.ToPort = ag.InputPortB
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			ag.InputPortB.FromPorts = append(ag.InputPortB.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	ag.OutputPort.OnClick = func() {
		fmt.Println("Output port of AND gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(ag.OutputPort.Pos, sim.CameraOffset),
			FromPort: ag.OutputPort,
		}
		sim.GhostObject = &wire
	}
	ag.InputPortA.Color.A = 128
	ag.InputPortB.Color.A = 128
	ag.OutputPort.Color.A = 128
	return ag
}

func (ag *AndGate) draw(CameraOffset *rl.Vector2) {
	Pos := rl.Vector2Subtract(ag.Pos, *CameraOffset)

	rl.DrawRectangleRec(
		rl.NewRectangle(Pos.X-20, Pos.Y-20, 20, 40),
		ag.Color,
	)
	rl.DrawCircleSector(Pos, 20, -90, 90, 32, ag.Color)

	ag.InputPortA.draw(CameraOffset)
	ag.InputPortB.draw(CameraOffset)
	ag.OutputPort.draw(CameraOffset)
}

func (ag *AndGate) update() {
}

func (ag *AndGate) isDynamic() bool {
	return false
}

func (ag *AndGate) setPosition(Position rl.Vector2) {
	ag.Pos = Position
	ag.InputPortA.Pos = rl.NewVector2(Position.X-26, Position.Y-10)
	ag.InputPortB.Pos = rl.NewVector2(Position.X-26, Position.Y+10)
	ag.OutputPort.Pos = rl.NewVector2(Position.X+26, Position.Y)
}

func (ag *AndGate) setTranslucent(set bool) {
	if set {
		ag.Color.A = 128
		ag.InputPortA.Color.A = 128
		ag.InputPortB.Color.A = 128
		ag.OutputPort.Color.A = 128
	} else {
		ag.Color.A = 255
		ag.InputPortA.Color.A = 255
		ag.InputPortB.Color.A = 255
		ag.OutputPort.Color.A = 255
	}
}

func (ag *AndGate) HandleInput() {
	ag.InputPortA.HandleInput()
	ag.InputPortB.HandleInput()
	ag.OutputPort.HandleInput()
}
