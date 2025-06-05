package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

// Visualization component: Arrow
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------

// Visualization component: XYGraph
// ----------------------------------------------------------------------
type XYGraph struct {
	Origin rl.Vector2
	Points []rl.Vector2
	Color  rl.Color
}

func (g *XYGraph) draw() {
	x := g.Origin.X - 100
	y := g.Origin.Y - 300
	x1 := g.Origin.X + 400
	y1 := g.Origin.Y + 300
	rl.DrawLine(int32(x), int32(g.Origin.Y), int32(x1), int32(g.Origin.Y), rl.DarkGray)
	rl.DrawLine(int32(g.Origin.X), int32(y), int32(g.Origin.X), int32(y1), rl.DarkGray)
	xi := 0
	for _, v := range g.Points {
		scaledY := g.Origin.Y - v.Y/5
		scaledX := g.Origin.X + float32(xi)
		xi++
		fmt.Println(scaledY)
		rl.DrawPixelV(rl.NewVector2(scaledX, float32(scaledY)), g.Color)
	}
}

// ----------------------------------------------------------------------
