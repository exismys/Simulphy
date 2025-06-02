package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type StateResMethod int

const (
	NONE StateResMethod = iota
	NOT
	OR
	AND
)

type Port struct {
	Id            int32
	Pos           rl.Vector2
	Radius        float32
	Color         rl.Color
	onClick       func()
	State         bool
	IsInputPort   bool
	FromPortsIds  []int32 // For serialization
	InputPortsIds []int32 // For serialization
	FromPorts     []*Port `json:"-"` // Not serializable
	InputPorts    []*Port `json:"-"` // Not serializable
	ResMethod     StateResMethod
	CameraOffset  rl.Vector2
}

func (p *Port) draw(cameraOffset *rl.Vector2) {
	p.CameraOffset = *cameraOffset
	radius := p.Radius
	if p.hovered() {
		radius += 2
	}
	rl.DrawCircleV(rl.Vector2Subtract(p.Pos, *cameraOffset), radius, p.Color)
}

func (p *Port) hovered() bool {
	mouse := rl.GetMousePosition()
	return rl.CheckCollisionPointCircle(mouse, rl.Vector2Subtract(p.Pos, p.CameraOffset), p.Radius)
}

func (p *Port) HandleInput() {
	if p.hovered() && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
		p.onClick()
		fmt.Printf("Address of clicked port (ID %d): %p", p.Id, p)
	}
}
