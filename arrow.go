package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Arrow struct {
	Pos1 rl.Vector2
	Pos2 rl.Vector2
}

func (a *Arrow) draw() {
	rl.DrawLine(int32(a.Pos1.X), int32(a.Pos1.Y), int32(a.Pos2.X), int32(a.Pos2.Y), rl.LightGray)

	tip := a.Pos2
	dir := rl.Vector2Normalize(rl.Vector2Subtract(a.Pos2, a.Pos1))

	dirX := dir.X
	dirY := dir.Y

	arrowHeadLength := float32(20)
	arrowHeadWidth := float32(10)

	left := rl.Vector2{
		X: a.Pos2.X + arrowHeadLength*(-dirX) + arrowHeadWidth*dirY,
		Y: a.Pos2.Y + arrowHeadLength*(-dirY) + arrowHeadWidth*(-dirX),
	}

	right := rl.Vector2{
		X: a.Pos2.X + arrowHeadLength*(-dirX) + arrowHeadWidth*(-dirY),
		Y: a.Pos2.Y + arrowHeadLength*(-dirY) + arrowHeadWidth*(dirX),
	}

	rl.DrawTriangle(tip, left, right, rl.LightGray)
}
