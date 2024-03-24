#!/usr/bin/env node


// Setup QUnit
//var q = require('qunitjs');
//console.log(q);
//QUnit.init();
//QUnit.config.blocking = false;
//QUnit.config.autorun = true;
//QUnit.config.updateRate = 0;
//QUnit.log = function(result) {
//console.log(result);
//console.log(result.result ? 'PASS' : 'FAIL', result.message);
//};



QUnit.test("first test", function( assert ) {
	assert.equal(1, 2, "1 should equal 2");
});

QUnit.test("first test", function( assert ) {
	assert.equal(1, 2, "1 should equal 2 a second time");
});

QUnit.test("first test", function( assert ) {
	assert.equal(1, 2, "1 should equal 2 a third time");
});

QUnit.test("first test", function( assert ) {
	assert.equal(1, 2, "1 should equal 2 a fourth time");
});

