package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Button struct {
	Pos     rl.Vector2
	Size    rl.Vector2
	Label   string
	onClick func()
}

func (b *Button) hovered() bool {
	mouse := rl.GetMousePosition()
	rect := rl.Rectangle{
		X:      b.Pos.X,
		Y:      b.Pos.Y,
		Width:  b.Size.X,
		Height: b.Size.Y,
	}
	return rl.CheckCollisionPointRec(mouse, rect)
}

func (b *Button) Draw() {
	bgColor := rl.DarkGray
	fgColor := rl.LightGray
	if b.hovered() {
		bgColor = rl.LightGray
		fgColor = rl.DarkGray
	}
	rect := rl.NewRectangle(b.Pos.X, b.Pos.Y, b.Size.X, b.Size.Y)
	// rl.DrawRectangle(int32(b.Pos.X), int32(b.Pos.Y), int32(b.Size.X), int32(b.Size.Y), bgColor)
	rl.DrawRectangleRounded(rect, 0.2, 32, bgColor)
	fontSize := 24
	labelWidth := rl.MeasureText(b.Label, int32(fontSize))
	rl.DrawText(b.Label, int32(b.Pos.X+(b.Size.X-float32(labelWidth))/2), int32(b.Pos.Y+(b.Size.Y-float32(fontSize))/2), int32(fontSize), fgColor)
}

func (b *Button) HandleInput() {
	if rl.IsMouseButtonPressed(rl.MouseButtonLeft) && b.hovered() {
		b.onClick()
	}
}
