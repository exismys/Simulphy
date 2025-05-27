package main

import rl "github.com/gen2brain/raylib-go/raylib"

type StateResMethod int

const (
	NONE StateResMethod = iota
	NOT
	OR
	AND
)

type Port struct {
	Pos          rl.Vector2
	Radius       float32
	Color        rl.Color
	onClick      func()
	State        bool
	IsInputPort  bool
	FromPorts    []*Port
	InputPorts   []*Port
	ResMethod    StateResMethod
	CameraOffset rl.Vector2
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
	}
}
