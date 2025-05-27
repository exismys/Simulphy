package main

import "fmt"

var wires []*Wire
var ports []*Port
var leds []*Led

var finalPort *Port

func calculateState(p *Port) bool {
	if p.IsInputPort {
		state := p.State
		for _, fp := range p.FromPorts {
			state = state || calculateState(fp)
		}
		fmt.Println("Returning state: ", state)
		return state
	}
	state := p.State
	if p.ResMethod == NOT {
		for _, ip := range p.InputPorts {
			state = !calculateState(ip)
		}
	} else if p.ResMethod == OR {
		state = false
		for _, ip := range p.InputPorts {
			state = state || calculateState(ip)
		}
	} else if p.ResMethod == AND {
		state = true
		for _, ip := range p.InputPorts {
			state = state && calculateState(ip)
		}
	} else if p.ResMethod == NONE {
	}
	fmt.Println("Returning state: ", state)
	return state
}

func refreshState() {
	for _, led := range leds {
		led.State = calculateState(led.InputPort)
	}
}
