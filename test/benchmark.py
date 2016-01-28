#! /usr/bin/env python3

import os
import shlex, subprocess
import time


def run(inputs, gates, health, observations, output_filename):
	filename = "dc-{}-{}".format(gates, health)

	# gendc.py
	# Usage: ./gendc.py <filename> <inputs> <gates> <health> <observations>
	gendc = "../data/models/dc/gendc.py {} {} {} {} {}".format(filename, inputs, gates, health, observations)
	print(gendc, end='\t')
	start = time.time()
	subprocess.call(shlex.split(gendc))
	end = time.time()
	uptime = end-start
	print("time = {}".format(round(uptime, 4)))

	# ./dbn
	# Usage: ../dbn /path/to/model.duai /path/to/observations.duai.evid [OPTIONS]
	dbn = "../dbn {f}.duai {f}.duai.evid -m 23".format(f=filename)
	print(dbn, end='\t')
	start = time.time()
	subprocess.call(shlex.split(dbn), stdout=open(output_filename, 'a'))
	end = time.time()
	uptime = end-start
	print("time = {}".format(round(uptime, 4)))
	print()

	os.remove(filename + ".duai")
	os.remove(filename + ".duai.evid")


def benchmark_sensor(observations, gates, health, models):
	print(">> Running benchmark_sensor ...")

	output_filename = "benchmarks-sensor.txt"
	if os.path.isfile(output_filename):
		os.remove(output_filename)

	gates = 17
	for inputs in range(3,gates+1):
		for i in range(models):
			run(inputs, gates, health, observations, output_filename)


def benchmark_interface(observations, inputs, models):
	print(">> Running benchmark_interface ...")

	output_filename = "benchmarks-interface.txt"
	if os.path.isfile(output_filename):
		os.remove(output_filename)

	gates = inputs * 2
	for health in range(5,gates+1):
		for i in range(models):
			run(inputs, gates, health, observations, output_filename)


def benchmark_timeslices(inputs, gates, health, models):
	print(">> Running benchmark_timeslices ...")

	output_filename = "benchmarks-timeslices.txt"
	if os.path.isfile(output_filename):
		os.remove(output_filename)

	for observations in range(50,501,50):
		for i in range(models):
			run(inputs, gates, health, observations, output_filename)


if __name__ == '__main__':

	# default parameters
	observations = 100
	inputs = 5
	gates = 20
	health = 5
	models = 10

	benchmark_sensor(observations, gates, health, models)
	benchmark_interface(observations, inputs, models)
	benchmark_timeslices(10, gates, 7, models)
