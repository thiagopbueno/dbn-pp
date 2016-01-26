#! /usr/bin/env python3

import random

def model_type():
	return "DBAYES"

def variables(inputs, gates, health):
	total = inputs + gates + 2*health
	model_vars = [total]
	for i in range(total):
		model_vars.append(2)
	return model_vars

def transition(inputs, gates, health):
	total = 2*health
	transition_vars = [total]
	i = 0
	for j in range(health):
		transition_vars.append(inputs + gates + i)
		transition_vars.append(inputs + gates + i+1)
		i += 2
	return transition_vars

def sensor(inputs, gates):
	total = inputs + 1
	sensor_vars = [total]
	for i in range(inputs):
		sensor_vars.append(i)
	sensor_vars.append(inputs+gates-1)
	return sensor_vars

def domain(inputs, gates, health):
	domains = []

	# inputs
	for i in range(inputs):
		domain = [1, i]
		domains.append(domain)

	# gates
	health_vars = list(range(health))
	random.shuffle(health_vars)

	health_index = 0
	for j in range(inputs, inputs+gates-1):
		domain = []

		# choose gate type [and, or, not]
		gate_type = random.randint(1, 3)
		fan_in = 1
		if gate_type == 1 or gate_type == 2:
			fan_in = 2

		# domain width
		domain.append(fan_in + 2)

		# gate output variable
		domain.append(j)

		if fan_in == 2:
			# first gate input from input wires
			gate_input = random.randint(0, inputs-1)
			domain.append(gate_input)

			# second gate input random
			gate_input = random.randint(0, j-1)
			while gate_input == domain[-1]:
				gate_input = random.randint(0, j-1)
			domain.append(gate_input)
		elif fan_in == 1:
			gate_input = random.randint(0, j-1)
			domain.append(gate_input)

		# gate health variable
		gate_health = health_vars[health_index]
		domain.append(inputs + gates + 2*gate_health)
		health_index = (health_index + 1) % health

		domains.append(domain)

	# last gate = output
	j = inputs+gates-1
	domain = [4, j, j-1, j-2]
	gate_health = health_vars[health_index]
	domain.append(inputs + gates + 2*gate_health)
	domains.append(domain)

	# health
	curr = True
	for k in range(inputs+gates, inputs+gates+2*health):
		domain = []

		if curr:
			domain += [1, k]
		else:
			domain += [2, k, k-1]

		curr = not curr

		domains.append(domain)

	return domains

def factors(inputs, gates, health, domains):
	factors_lst = []

	# inputs
	for i in range(inputs):
		p = random.random()
		factor = [2, p, 1-p]
		factors_lst.append(factor)

	# gates
	for j in range(inputs, inputs+gates):
		domain = domains[j]
		width = domain[0]
		size = 2 ** (len(domain)-1)

		factor = [size]

		gate_type = ""
		if width == 3:
			gate_type = "not"
		elif width == 4:
			gate_type = random.choice(["and", "or"])

		instantiation = [0]*width
		for l in range(size):
			if instantiation[-1] == 0:
				factor.append(0.5)
			else:
				if gate_type == "not":
					if bool(instantiation[0]) == (not bool(instantiation[1])):
						factor.append(1.0)
					else:
						factor.append(0.0)
				elif gate_type == "and":
					if bool(instantiation[0]) == (bool(instantiation[1]) and bool(instantiation[2])):
						factor.append(1.0)
					else:
						factor.append(0.0)
				elif gate_type == "or":
					if bool(instantiation[0]) == (bool(instantiation[1]) or bool(instantiation[2])):
						factor.append(1.0)
					else:
						factor.append(0.0)

			# next instantiation
			for d in range(width-1, -1, -1):
				if instantiation[d] == 1:
					instantiation[d] = 0
				else:
					instantiation[d] = 1
					break

		factors_lst.append(factor)

	# health
	curr = True
	for k in range(inputs+gates, inputs+gates+2*health):
		factor = []

		if curr:
			p = random.random()
			factor = [2, p, 1-p]
		else:
			p1 = random.random()
			p2 = random.random()
			factor = [4, p1, p2, 1-p1, 1-p2]
		
		factors_lst.append(factor)

		curr = not curr

	return factors_lst

if __name__ == '__main__':
	import sys
	import functools

	if len(sys.argv) < 6:
		print("usage: {} <filename> <inputs> <gates> <health> <observations>".format(sys.argv[0]))
		exit(-1)

	# arguments
	filename = sys.argv[1]
	inputs = int(sys.argv[2])
	gates = int(sys.argv[3])
	health = int(sys.argv[4])
	observations = int(sys.argv[5])

	model_vars = list(map(str, variables(inputs, gates, health)))
	transition_vars = list(map(str, transition(inputs, gates, health)))
	sensor_vars = list(map(str, sensor(inputs, gates)))
	domains = domain(inputs, gates, health)
	factors_lst = factors(inputs, gates, health, domains)

	with open(filename + '.duai', mode='w', encoding='utf-8') as model:
		model.write(model_type() + "\n\n")

		model.write("# Variables\n")
		model.write(model_vars[0] + "\n")
		model.write(' '.join(model_vars[1:]) + "\n\n")

		model.write("# Interface\n")
		model.write(transition_vars[0] + "\n")
		model.write(' '.join(transition_vars[1:]) + "\n\n")

		model.write("# Observations\n")
		model.write(sensor_vars[0] + "\n")
		model.write(' '.join(sensor_vars[1:]) + "\n\n")

		model.write("# Domains\n")
		for domain in domains:
			domain_vars = list(map(str, domain))
			model.write(' '.join(domain_vars) + "\n")
		model.write("\n")

		model.write("# Factors\n")
		for factor in factors_lst:
			factor_values = list(map(str, factor))
			model.write(' '.join(factor_values) + "\n")
		model.write("\n")

	with open(filename + '.duai.evid', mode='w', encoding='utf-8') as evidence:
		nsensors = str(sensor_vars[0])
		nobservations = str(observations)
		evidence.write("{} {}\n".format(nsensors, nobservations))

		for sensor_id in sensor_vars[1:]:
			evidence.write(str(sensor_id))
			for i in range(observations):
				evidence.write(" {}".format(str(random.choice([0,1]))))
			evidence.write("\n")

		evidence.write(str(int(transition_vars[0])//2))
		for var_id in transition_vars[1::2]:
			evidence.write(" {}".format(str(var_id)))
		evidence.write("\n")
