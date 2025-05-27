package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

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
