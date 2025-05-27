package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type NotGate struct {
	Pos        rl.Vector2
	Color      rl.Color
	InputPort  *Port
	OutputPort *Port
}

func NewNotGate(sim *Simulation, Position rl.Vector2, Color rl.Color) *NotGate {
	ng := &NotGate{
		Pos:   Position,
		Color: Color,
	}
	ng.InputPort = &Port{
		Pos:         rl.NewVector2(ng.Pos.X-26, ng.Pos.Y),
		Radius:      5,
		Color:       rl.SkyBlue,
		IsInputPort: true,
		FromPorts:   []*Port{},
	}
	ng.OutputPort = &Port{
		Pos:        rl.NewVector2(ng.Pos.X+28, ng.Pos.Y),
		Radius:     5,
		Color:      rl.Orange,
		InputPorts: []*Port{ng.InputPort},
		ResMethod:  NOT,
	}
	ng.InputPort.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = ng.InputPort.Pos
			w.ToPort = ng.InputPort
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			ng.InputPort.FromPorts = append(ng.InputPort.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	ng.OutputPort.onClick = func() {
		fmt.Println("Output port of NOT gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(ng.OutputPort.Pos, sim.CameraOffset),
			FromPort: ng.OutputPort,
		}
		sim.GhostObject = &wire
	}
	ng.InputPort.Color.A = 128
	ng.OutputPort.Color.A = 128
	return ng
}

func (ng *NotGate) draw(CameraOffset *rl.Vector2) {
	Pos := rl.Vector2Subtract(ng.Pos, *CameraOffset)

	p1 := rl.NewVector2(Pos.X-20, Pos.Y-20)
	p2 := rl.NewVector2(Pos.X-20, Pos.Y+20)
	p3 := rl.NewVector2(Pos.X+10, Pos.Y)

	rl.DrawTriangle(p1, p2, p3, ng.Color)
	rl.DrawCircle(int32(Pos.X+16), int32(Pos.Y), 6, ng.Color)

	ng.InputPort.draw(CameraOffset)
	ng.OutputPort.draw(CameraOffset)
}

func (ng *NotGate) update() {
}

func (ng *NotGate) isDynamic() bool {
	return false
}

func (ng *NotGate) setPosition(Position rl.Vector2) {
	ng.Pos = Position
	ng.InputPort.Pos = rl.NewVector2(Position.X-26, Position.Y)
	ng.OutputPort.Pos = rl.NewVector2(Position.X+28, Position.Y)
}

func (ng *NotGate) setTranslucent(set bool) {
	if set {
		ng.Color.A = 128
		ng.InputPort.Color.A = 128
		ng.OutputPort.Color.A = 128
	} else {
		ng.Color.A = 255
		ng.InputPort.Color.A = 255
		ng.OutputPort.Color.A = 255
	}
}

func (ng *NotGate) HandleInput() {
	ng.InputPort.HandleInput()
	ng.OutputPort.HandleInput()
}
