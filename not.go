package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

type NotGate struct {
	pos        rl.Vector2
	color      rl.Color
	inputPort  *Port
	outputPort *Port
}

func NewNotGate(position rl.Vector2, color rl.Color) *NotGate {
	ng := &NotGate{
		pos:   position,
		color: color,
	}
	ng.inputPort = &Port{
		pos:    rl.NewVector2(ng.pos.X-26, ng.pos.Y),
		radius: 5,
		color:  rl.SkyBlue,
	}
	ng.outputPort = &Port{
		pos:    rl.NewVector2(ng.pos.X+28, ng.pos.Y),
		radius: 5,
		color:  rl.Orange,
	}
	ng.inputPort.color.A = 128
	ng.outputPort.color.A = 128
	return ng
}

func (ng *NotGate) draw(cameraOffset *rl.Vector2) {
	pos := rl.Vector2Subtract(ng.pos, *cameraOffset)

	p1 := rl.NewVector2(pos.X-20, pos.Y-20)
	p2 := rl.NewVector2(pos.X-20, pos.Y+20)
	p3 := rl.NewVector2(pos.X+10, pos.Y)

	rl.DrawTriangle(p1, p2, p3, ng.color)
	rl.DrawCircle(int32(pos.X+16), int32(pos.Y), 6, ng.color)

	ng.inputPort.draw(cameraOffset)
	ng.outputPort.draw(cameraOffset)
}

func (ng *NotGate) update() {
}

func (ng *NotGate) isDynamic() bool {
	return false
}

func (ng *NotGate) isClicked() bool {
	return false
}

func (ng *NotGate) setPosition(position rl.Vector2) {
	ng.pos = position
	ng.inputPort.pos = rl.NewVector2(position.X-26, position.Y)
	ng.outputPort.pos = rl.NewVector2(position.X+28, position.Y)
}

func (ng *NotGate) setTranslucent(set bool) {
	if set {
		ng.color.A = 128
		ng.inputPort.color.A = 128
		ng.outputPort.color.A = 128
	} else {
		ng.color.A = 255
		ng.inputPort.color.A = 255
		ng.outputPort.color.A = 255
	}
}

func (ng *NotGate) HandleInput() {
	ng.inputPort.HandleInput()
	ng.outputPort.HandleInput()
}
