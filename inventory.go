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
}

func (inv *Inventory) Draw() {
	if !inv.Visible {
		return
	}

	initPos := inv.Pos

	for _, item := range inv.Items {
		initPos.Y += float32(inv.ItemHeight)
		btn := &Button{
			Pos:   initPos,
			Size:  rl.NewVector2(100, 50),
			Label: item,
			OnClick: func() {
				fmt.Println("Clicked ", item)
				if inv.OnSelect != nil {
					inv.OnSelect(item)
				}
			},
		}
		inv.buttons = append(inv.buttons, btn)
		fmt.Println(len(inv.buttons))
		btn.Draw()
	}
}

func (inv *Inventory) HandleInput() {
	for _, btn := range inv.buttons {
		btn.HandleInput()
	}
}
