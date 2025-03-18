package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Grid struct {
	origin    rl.Vector2
	length    float32
	width     float32
	lineColor rl.Color
}

// func (g *Grid) draw() {
// 	// Horizontal lines
// 	for y := g.origin.Y; y >= 0; y -= float32(pixelsPerMeter) {
// 		rl.DrawLine(0, int32(y), simWidth-1, int32(y), g.lineColor)
// 	}
// 	for y := g.origin.Y + float32(pixelsPerMeter); y < float32(simHeight); y += float32(pixelsPerMeter) {
// 		rl.DrawLine(0, int32(y), simWidth-1, int32(y), g.lineColor)
// 	}

// 	// Vertical lines
// 	for x := g.origin.X; x >= 0; x -= float32(pixelsPerMeter) {
// 		rl.DrawLine(int32(x), 0, int32(x), simHeight-1, g.lineColor)
// 	}
// 	for x := g.origin.X + float32(pixelsPerMeter); x < float32(simWidth); x += float32(pixelsPerMeter) {
// 		rl.DrawLine(int32(x), 0, int32(x), simHeight-1, g.lineColor)
// 	}
// }
