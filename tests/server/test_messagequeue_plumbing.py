from unittest import TestCase

class TestJoin(TestCase):
    """A worker wants to join the hive"""
    pass

class TestLeaveAfterJoin(TestCase):
    """A worker wants to leave after it joined"""
    pass

class TestDoubleJoin(TestCase):
    """A worker tries to join after it already joined"""
    pass

class TestLeaveWithoutJoin(TestCase):
    """A worker tries to leave before it joined"""
    pass
