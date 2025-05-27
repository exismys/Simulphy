package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

type Circle struct {
	Pos, Vel, Acc rl.Vector2
	Radius        float32
	Color         rl.Color
}

func (c *Circle) draw(cameraOffset *rl.Vector2) {
	rl.DrawCircle(int32(c.Pos.X-cameraOffset.X), int32(c.Pos.Y-cameraOffset.Y), float32(c.Radius), c.Color)
}

func (c *Circle) update() {
	deltaTime := fixedDeltaTime

	// Calculate Velocity
	c.Vel.X += c.Acc.X * deltaTime
	c.Vel.Y += c.Acc.Y * deltaTime

	// Calculate Position
	c.Pos.X += c.Vel.X * deltaTime
	c.Pos.Y += c.Vel.Y * deltaTime

	// Handle Collision
	if c.Pos.X-c.Radius < 0 {
		c.Pos.X = 0 + c.Radius
		c.Vel.X *= -1 * dampingFactor
	} else if c.Pos.X+c.Radius > float32(simWidth) {
		c.Pos.X = float32(simWidth) - c.Radius
		c.Vel.X *= -1 * dampingFactor
	}

	if c.Pos.Y-c.Radius < 0 {
		c.Pos.Y = 0 + c.Radius
		c.Vel.Y *= -1 * dampingFactor
	} else if c.Pos.Y+c.Radius > float32(simHeight) {
		c.Pos.Y = float32(simHeight) - c.Radius
		c.Vel.Y *= -1 * dampingFactor
	}
}

func (c *Circle) isDynamic() bool {
	return true
}

func (c *Circle) setPosition(Position rl.Vector2) {
	c.Pos = Position
}

func (c *Circle) setTranslucent(set bool) {
	if set {
		c.Color.A = 128
	} else {
		c.Color.A = 255
	}
}

func (c *Circle) HandleInput() {

}
