package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

type NotGate struct {
	pos   rl.Vector2
	color rl.Color
}

func (ng *NotGate) draw(cameraOffset *rl.Vector2) {
	pos := rl.Vector2Subtract(ng.pos, *cameraOffset)

	p1 := rl.NewVector2(pos.X-20, pos.Y-20)
	p2 := rl.NewVector2(pos.X-20, pos.Y+20)
	p3 := rl.NewVector2(pos.X+10, pos.Y)

	rl.DrawTriangle(p1, p2, p3, ng.color)
	rl.DrawCircle(int32(pos.X+16), int32(pos.Y), 6, ng.color)
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
}

func (ng *NotGate) setTranslucent(set bool) {
	if set {
		ng.color.A = 128
	} else {
		ng.color.A = 255
	}
}
