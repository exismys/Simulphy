package main

import "fmt"

func calculateState(p *Port) bool {
	if p.IsInputPort {
		state := p.State
		fmt.Println("current port state: ", p.State, p.Id, p)
		for _, fp := range p.FromPorts {
			fmt.Println("From Port states", fp.State, fp.Id, fp)
			fmt.Printf("memory of from port %p", fp)
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
