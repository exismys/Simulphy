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
	OnSelect   func(item string)
	ItemHeight int
	ItemWidth  int
}

func NewInventory(pos rl.Vector2, items []string, itemHeight int, itemWidth int, onSelect func(item string)) *Inventory {
	inv := &Inventory{
		Pos:        pos,
		Visible:    false,
		Items:      items,
		OnSelect:   onSelect,
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
	initPos := inv.Pos
	for _, item := range inv.Items {
		initPos.Y -= float32(inv.ItemHeight)
		btn := &Button{
			Pos:   initPos,
			Size:  rl.NewVector2(float32(inv.ItemWidth), float32(inv.ItemHeight)),
			Label: item,
			OnClick: func() {
				fmt.Println("Clicked ", item)
				if inv.OnSelect != nil {
					inv.OnSelect(item)
				}
			},
		}
		inv.buttons = append(inv.buttons, btn)
	}
}
