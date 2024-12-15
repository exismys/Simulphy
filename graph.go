package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type XYGraph struct {
	origin rl.Vector2
	points []rl.Vector2
}

func (g *XYGraph) draw() {
	x := g.origin.X - 100
	y := g.origin.Y - 300
	x1 := g.origin.X + 400
	y1 := g.origin.Y + 300
	rl.DrawLine(int32(x), int32(g.origin.Y), int32(x1), int32(g.origin.Y), rl.DarkGray)
	rl.DrawLine(int32(g.origin.X), int32(y), int32(g.origin.X), int32(y1), rl.DarkGray)
	xi := 0
	for _, v := range g.points {
		scaledY := g.origin.Y - v.Y/5
		scaledX := g.origin.X + float32(xi)
		xi++
		fmt.Println(scaledY)
		rl.DrawPixelV(rl.NewVector2(scaledX, float32(scaledY)), rl.Red)
	}
}
