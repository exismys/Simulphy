package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type Power struct {
	pos          rl.Vector2
	outputPort   *Port
	state        bool
	radiusInner  float32
	radiusOuter  float32
	cameraOffset rl.Vector2
	// Color specifications
	colorOuter      rl.Color
	colorInnerTrue  rl.Color
	colorInnerFalse rl.Color
}

func NewPowerSource(sim *Simulation, position rl.Vector2) *Power {
	p := &Power{
		pos:             position,
		state:           true,
		radiusOuter:     20,
		radiusInner:     15,
		colorOuter:      rl.LightGray,
		colorInnerTrue:  rl.Red,
		colorInnerFalse: rl.Gray,
	}
	p.outputPort = &Port{
		pos:    rl.NewVector2(p.pos.X+p.radiusOuter+6, p.pos.Y),
		radius: 5,
		color:  rl.Orange,
	}
	p.outputPort.onClick = func() {
		fmt.Println("Output port of POWER SOURCE clicked")
		wire := Wire{
			From: rl.Vector2Subtract(p.outputPort.pos, sim.cameraOffset),
		}
		sim.ghostObject = &wire
	}
	p.outputPort.color.A = 128
	return p
}

func (p *Power) draw(cameraOffset *rl.Vector2) {
	rl.DrawCircleV(rl.Vector2Subtract(p.pos, *cameraOffset), p.radiusOuter, p.colorOuter)
	stateColor := p.colorInnerFalse
	if p.state {
		stateColor = p.colorInnerTrue
	}
	innerRadius := p.radiusInner
	if p.hovered() {
		innerRadius += 2
	}
	rl.DrawCircleV(rl.Vector2Subtract(p.pos, *cameraOffset), innerRadius, stateColor)
	p.outputPort.draw(cameraOffset)
}

func (p *Power) update() {
}

func (p *Power) HandleInput() {
	if p.hovered() && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
		p.state = !p.state
	}
	p.outputPort.HandleInput()
}

func (p *Power) setTranslucent(set bool) {
	if set {
		p.colorInnerFalse.A = 128
		p.colorInnerTrue.A = 128
		p.colorOuter.A = 128
		p.outputPort.color.A = 128
	} else {
		p.colorInnerFalse.A = 255
		p.colorInnerTrue.A = 255
		p.colorOuter.A = 255
		p.outputPort.color.A = 255
	}
}

func (p *Power) isDynamic() bool {
	return false
}

func (p *Power) setPosition(position rl.Vector2) {
	p.pos = position
	p.outputPort.pos = rl.NewVector2(position.X+p.radiusOuter+6, position.Y)
}

func (p *Power) hovered() bool {
	mouse := rl.GetMousePosition()
	return rl.CheckCollisionPointCircle(mouse, rl.Vector2Subtract(p.pos, p.cameraOffset), p.radiusInner)
}
