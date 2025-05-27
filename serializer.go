package main

import (
	"encoding/json"
	"fmt"
	"os"
)

type SimState struct {
	Sim   *Simulation
	Wires []*Wire
	Ports []*Port
	Leds  []*Led
}

func serialize(sim *Simulation) {
	simState := &SimState{
		Sim:   sim,
		Wires: wires,
		Ports: ports,
		Leds:  leds,
	}

	s, error := json.Marshal(simState)
	if error != nil {
		fmt.Println(error)
	}
	os.WriteFile("circuit.sim.json", s, 0644)
}
