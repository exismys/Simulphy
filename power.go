package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type Power struct {
	Pos          rl.Vector2
	OutputPort   *Port
	State        bool
	RadiusInner  float32
	RadiusOuter  float32
	cameraOffset rl.Vector2
	// Color specifications
	ColorOuter      rl.Color
	ColorInnerTrue  rl.Color
	ColorInnerFalse rl.Color
}

func NewPowerSource(sim *Simulation, Position rl.Vector2) *Power {
	p := &Power{
		Pos:             Position,
		State:           true,
		RadiusOuter:     20,
		RadiusInner:     15,
		ColorOuter:      rl.LightGray,
		ColorInnerTrue:  rl.Red,
		ColorInnerFalse: rl.Gray,
	}
	p.OutputPort = &Port{
		Pos:         rl.NewVector2(p.Pos.X+p.RadiusOuter+6, p.Pos.Y),
		Radius:      5,
		Color:       rl.Orange,
		State:       p.State,
		ResMethod:   NONE,
		IsInputPort: false,
		InputPorts:  []*Port{},
	}
	p.OutputPort.onClick = func() {
		fmt.Println("Output port of POWER SOURCE clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(p.OutputPort.Pos, sim.CameraOffset),
			FromPort: p.OutputPort,
		}
		sim.GhostObject = &wire
	}
	p.OutputPort.Color.A = 128
	return p
}

func (p *Power) draw(cameraOffset *rl.Vector2) {
	p.cameraOffset = *cameraOffset
	rl.DrawCircleV(rl.Vector2Subtract(p.Pos, *cameraOffset), p.RadiusOuter, p.ColorOuter)
	StateColor := p.ColorInnerFalse
	if p.State {
		StateColor = p.ColorInnerTrue
	}
	innerRadius := p.RadiusInner
	if p.hovered() {
		innerRadius += 2
	}
	rl.DrawCircleV(rl.Vector2Subtract(p.Pos, *cameraOffset), innerRadius, StateColor)
	p.OutputPort.draw(cameraOffset)
}

func (p *Power) update() {
}

func (p *Power) HandleInput() {
	if p.hovered() && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
		p.State = !p.State
		p.OutputPort.State = p.State
		refreshState()
	}
	p.OutputPort.HandleInput()
}

func (p *Power) setTranslucent(set bool) {
	if set {
		p.ColorInnerFalse.A = 128
		p.ColorInnerTrue.A = 128
		p.ColorOuter.A = 128
		p.OutputPort.Color.A = 128
	} else {
		p.ColorInnerFalse.A = 255
		p.ColorInnerTrue.A = 255
		p.ColorOuter.A = 255
		p.OutputPort.Color.A = 255
	}
}

func (p *Power) isDynamic() bool {
	return false
}

func (p *Power) setPosition(Position rl.Vector2) {
	p.Pos = Position
	p.OutputPort.Pos = rl.NewVector2(Position.X+p.RadiusOuter+6, Position.Y)
}

func (p *Power) hovered() bool {
	mouse := rl.GetMousePosition()
	return rl.CheckCollisionPointCircle(mouse, rl.Vector2Subtract(p.Pos, p.cameraOffset), p.RadiusInner)
}
