package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Port struct {
	pos          rl.Vector2
	radius       float32
	color        rl.Color
	onClick      func()
	cameraOffset rl.Vector2
}

func (p *Port) draw(cameraOffset *rl.Vector2) {
	p.cameraOffset = *cameraOffset
	radius := p.radius
	if p.hovered() {
		radius += 2
	}
	rl.DrawCircleV(rl.Vector2Subtract(p.pos, *cameraOffset), radius, p.color)
}

func (p *Port) hovered() bool {
	mouse := rl.GetMousePosition()
	return rl.CheckCollisionPointCircle(mouse, rl.Vector2Subtract(p.pos, p.cameraOffset), p.radius)
}

func (p *Port) HandleInput() {
	if p.hovered() && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
		p.onClick()
	}
}
