package main

import (
	"encoding/json"
	"fmt"
	"os"
)

type SimState struct {
	// Sim   *Simulation
	Wires []*Wire
	Ports []*Port
	Leds  []*Led
}

func serialize(sim *Simulation) {
	simState := &SimState{
		// Sim:   sim,
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

func deserialize(sim *Simulation) {
	data, err := os.ReadFile("circuit.sim.json")
	if err != nil {
		fmt.Println(err)
		return
	}

	simState := &SimState{}
	err = json.Unmarshal(data, simState)
	if err != nil {
		fmt.Println(err)
		return
	}

	ports = simState.Ports
	wires = simState.Wires
	leds = simState.Leds

	for _, led := range leds {
		sim.Objects = append(sim.Objects, led)
	}
}
