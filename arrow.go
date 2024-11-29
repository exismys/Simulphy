package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Arrow struct {
	pos1 rl.Vector2
	pos2 rl.Vector2
}

func (a *Arrow) draw() {
	rl.DrawLine(int32(a.pos1.X), int32(a.pos1.Y), int32(a.pos2.X), int32(a.pos2.Y), rl.LightGray)

	tip := a.pos2
	dir := rl.Vector2Normalize(rl.Vector2Subtract(a.pos2, a.pos1))

	dirX := dir.X
	dirY := dir.Y

	arrowHeadLength := float32(20)
	arrowHeadWidth := float32(10)

	left := rl.Vector2{
		X: a.pos2.X + arrowHeadLength*(-dirX) + arrowHeadWidth*dirY,
		Y: a.pos2.Y + arrowHeadLength*(-dirY) + arrowHeadWidth*(-dirX),
	}

	right := rl.Vector2{
		X: a.pos2.X + arrowHeadLength*(-dirX) + arrowHeadWidth*(-dirY),
		Y: a.pos2.Y + arrowHeadLength*(-dirY) + arrowHeadWidth*(dirX),
	}

	rl.DrawTriangle(tip, left, right, rl.LightGray)
}
