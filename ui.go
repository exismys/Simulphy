package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

// UI component: Button
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------

// UI component: Inventory
// ----------------------------------------------------------------------
type Inventory struct {
	Pos        rl.Vector2
	Visible    bool
	Items      []string
	buttons    []*Button
	onSelect   func(item string)
	ItemHeight int
	ItemWidth  int
}

func NewInventory(pos rl.Vector2, items []string, itemHeight int, itemWidth int, onSelect func(item string)) *Inventory {
	inv := &Inventory{
		Pos:        pos,
		Visible:    false,
		Items:      items,
		onSelect:   onSelect,
		ItemHeight: itemHeight,
		ItemWidth:  itemWidth,
	}
	inv.buildButtons()
	return inv
}

func (inv *Inventory) Draw() {
	if !inv.Visible {
		return
	}
	for _, btn := range inv.buttons {
		btn.Draw()
	}
}

func (inv *Inventory) HandleInput() {
	for _, btn := range inv.buttons {
		btn.HandleInput()
	}
}

func (inv *Inventory) buildButtons() {
	// Get the top-left position of the button as inv.Pos.Y is bottom-left
	initPos := rl.NewVector2(inv.Pos.X, inv.Pos.Y-float32(inv.ItemHeight))

	verticalGap := 10
	add := -(float32(inv.ItemHeight) + float32(verticalGap))
	for _, item := range inv.Items {
		if initPos.Y <= 20 {
			initPos.X += float32(inv.ItemWidth)
			initPos.Y = inv.Pos.Y - float32(inv.ItemHeight)
		}
		btn := &Button{
			Pos:   initPos,
			Size:  rl.NewVector2(float32(inv.ItemWidth), float32(inv.ItemHeight)),
			Label: item,
			onClick: func() {
				fmt.Println("Clicked ", item)
				if inv.onSelect != nil {
					inv.onSelect(item)
				}
			},
		}
		inv.buttons = append(inv.buttons, btn)
		initPos.Y += add
	}
}

// ----------------------------------------------------------------------

// UI component: TextBox
// ----------------------------------------------------------------------
type TextBox struct {
	Pos        rl.Vector2
	Visible    bool
	saveBtn    *Button
	onSave     func(item string)
	ItemHeight int
	ItemWidth  int
}

func NewTextBox(pos rl.Vector2, items []string, itemHeight int, itemWidth int, onSave func(item string)) *TextBox {
	tb := &TextBox{
		Pos:        pos,
		Visible:    false,
		onSave:     onSave,
		ItemHeight: itemHeight,
		ItemWidth:  itemWidth,
	}
	tb.buildSaveButton()
	return tb
}

func (tb *TextBox) Draw() {
	if !tb.Visible {
		return
	}
	tb.saveBtn.Draw()
}

func (inv *TextBox) HandleInput() {
}

func (tb *TextBox) buildSaveButton() {
}
