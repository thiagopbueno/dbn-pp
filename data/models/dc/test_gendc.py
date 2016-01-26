#! /usr/bin/env python3

import gendc
import unittest

class TestGendc(unittest.TestCase):

	@classmethod
	def setUpClass(cls):
		cls.inputs = 2
		cls.gates = 2
		cls.health = 2
		cls.total_number_vars = cls.inputs + cls.gates + 2*cls.health
		cls.model_type = gendc.model_type()
		cls.model_vars = gendc.variables(cls.inputs, cls.gates, cls.health)
		cls.prior_vars = gendc.prior(cls.inputs, cls.gates, cls.health)
		cls.transition_vars = gendc.transition(cls.inputs, cls.gates, cls.health)
		cls.sensor_vars = gendc.sensor(cls.inputs, cls.gates)
		cls.domains = gendc.domain(cls.inputs, cls.gates, cls.health)
		cls.factors = gendc.factors(cls.inputs, cls.gates, cls.health, cls.domains)

	def test_dbayes(self):
		self.assertEqual(self.model_type, "DBAYES")
		self.assertEqual(len(self.model_vars), self.total_number_vars+1)
		for cardinality in self.model_vars[1:]:
			self.assertEqual(cardinality, 2)

	def test_prior(self):
		self.assertEqual(self.prior_vars[0], self.health)
		self.assertEqual(len(self.prior_vars), self.health+1)
		i = 1
		for var_id in range(self.inputs + self.gates, len(self.prior_vars), 2):
			self.assertEqual(self.prior_vars[i], var_id)
			i += 1

	def test_transition(self):
		self.assertEqual(self.transition_vars[0], 2*self.health)
		self.assertEqual(len(self.transition_vars), 2*self.health+1)
		i = 1
		for var_id in range(self.inputs + self.gates, len(self.transition_vars), 2):
			self.assertEqual(self.transition_vars[i], var_id)
			self.assertEqual(self.transition_vars[i+1], var_id+1)
			i += 1

	def test_sensor(self):
		total_wires = self.inputs + 1
		self.assertEqual(self.sensor_vars[0], total_wires)
		self.assertEqual(len(self.sensor_vars), total_wires+1)
		for i in range(1, self.inputs+1):
			self.assertEqual(self.sensor_vars[i], i-1)

		gates = set()
		for j in range(self.inputs+1, len(self.sensor_vars)):
			self.assertTrue(self.inputs <= self.sensor_vars[j] < self.inputs+self.gates)
			self.assertFalse(self.sensor_vars[j] in gates)
			gates.add(self.sensor_vars[j])

	def test_domain(self):

		self.assertEqual(len(self.domains), self.total_number_vars)

		# inputs
		for i in range(self.inputs):
			domain = self.domains[i]
			self.assertEqual(len(domain), 2)
			self.assertEqual(domain[0], 1)
			self.assertEqual(domain[1], i)

		# gates
		health_vars = self.transition_vars[1::2]
		for j in range(self.inputs, self.inputs + self.gates):
			domain = self.domains[j]
			self.assertEqual(len(domain), domain[0]+1)
			self.assertEqual(domain[1], j)
			domain_vars = set()
			for k in range(2, len(domain)):
				self.assertTrue(domain[k] < j or domain[k] in health_vars)
				self.assertTrue(not domain[k] in domain_vars)
				domain_vars.add(domain[k])

		# health
		l = 0
		for k in range(self.health):

			curr_id =  self.inputs + self.gates + l
			domain_curr = self.domains[curr_id]
			self.assertEqual(domain_curr[0], 1)
			self.assertEqual(domain_curr[1], curr_id)

			next_id = self.inputs + self.gates + l+1
			domain_next = self.domains[next_id]
			self.assertEqual(domain_next[0], 2)
			self.assertEqual(domain_next[1], next_id)
			self.assertEqual(domain_next[2], curr_id)

			l += 2

	def test_factors(self):
		self.assertEqual(len(self.factors), self.total_number_vars)

		# inputs
		for i in range(self.inputs):
			factor = self.factors[i]

			self.assertEqual(len(factor), 3)
			self.assertEqual(factor[0], 2)
			self.assertTrue(all([p >= 0 for p in factor[1:]]))
			self.assertAlmostEqual(sum(factor[1:]), 1.0)

		# gates
		for j in range(self.inputs, self.inputs + self.gates):
			domain = self.domains[j]
			width = domain[0]
			factor = self.factors[j]
			size = factor[0]

			self.assertEqual(len(factor), 2**width + 1)
			self.assertEqual(size, 2**width)
			self.assertEqual(sum(factor[1:]), size/2)
			self.assertTrue(all([p >= 0 for p in factor[1:]]))

			healthy = False
			for k in range(1, size//2 + 1):
				if not healthy:
					self.assertAlmostEqual(factor[k], 0.5)
					self.assertAlmostEqual(factor[k+size//2], 0.5)
				self.assertAlmostEqual(factor[k] + factor[k+size//2], 1.0)
				healthy = not healthy

		# health
		l = 0
		for k in range(self.health):
			factor = self.factors[self.inputs + self.gates + l]
			self.assertEqual(len(factor), 3)
			self.assertEqual(factor[0], 2)
			self.assertTrue(factor[1] >= 0)
			self.assertAlmostEqual(factor[2], 1-factor[1])

			factor = self.factors[self.inputs + self.gates + l+1]
			domain = self.domains[self.inputs + self.gates + l+1]
			width = domain[0]
			size = factor[0]

			self.assertEqual(size, 2**width)
			self.assertEqual(len(factor), size+1)
			for k in range(1, size//2 + 1):
				self.assertTrue(factor[k] >= 0)
				self.assertTrue(factor[k+size//2] >= 0)
				self.assertAlmostEqual(factor[k] + factor[k+size//2], 1.0)

			l += 2

if __name__ == '__main__':
	unittest.main(verbosity=2)
