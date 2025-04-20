package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Port struct {
	pos    rl.Vector2
	radius float32
	color  rl.Color
}

func (p *Port) draw(cameraOffset *rl.Vector2) {
	rl.DrawCircleV(rl.Vector2Subtract(p.pos, *cameraOffset), p.radius, p.color)
}
