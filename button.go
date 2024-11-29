package main

import rl "github.com/gen2brain/raylib-go/raylib"

type TextButton struct {
	pos    rl.Vector2
	width  float32
	height float32
	text   string
}

func (b *TextButton) draw() {
	if b.width == 0 {
		b.width = 60
	}
	if b.height == 0 {
		b.height = 20
	}
	rl.DrawRectangleLines(int32(b.pos.X), int32(b.pos.Y), int32(b.width), int32(b.height), rl.LightGray)
	textWidth := rl.MeasureText(b.text, 10)
	textX := int32(b.pos.X + (b.width-float32(textWidth))/2)
	textY := int32(b.pos.Y + 5)
	rl.DrawText(b.text, textX, textY, 10, rl.LightGray)
}

func (b *TextButton) isClicked() bool {
	mousePos := rl.GetMousePosition()
	if rl.IsMouseButtonPressed(rl.MouseLeftButton) && mousePos.X > b.pos.X && mousePos.X < b.pos.X+b.width && mousePos.Y > b.pos.Y && mousePos.Y < b.pos.Y+b.height {
		return true
	}
	return false
}
