#!/usr/bin/env node
var argv = require('minimist')(process.argv.slice(2));
var pop;

process.stdin.setEncoding('utf8');
process.stdin.on('readable', handle_command);

// Main
process.nextTick(function () {
	pop = new population(new bridge_breeder(function () { return new bridge_species(); }));
	pop.run(completion_report, argv);
});

function completion_report() {
	console.log('' + pop.winner());
	pop.winner().dump_score();
	process.exit();
}

function handle_command() {
	var chunk = process.stdin.read();
	if (chunk !== null) {
		// process.stdout.write('Read: ' + chunk);
		console.log('Top score after ' + pop.iterations + ' iterations: ' + pop.winner().score());
	}
}


function population(new_species_factory) {
	this.new_species_factory = new_species_factory;
	this.iterations = 0;
}
population.prototype.setup = function (n) {
	this.maxpop = n;
	this.pop = [];
	for (i=0; i<n; i++) {
		this.insert(this.new_species_factory.build());
	}
}
population.prototype.winner = function () {
	return this.pop[0];
}
population.prototype.insert = function (species) {
	var pos = 0;
	var score = species.score();
	while (pos < this.pop.length-1 && this.pop[pos] && score > this.pop[pos].score()) pos++;
	if (pos < this.maxpop) {
		// Push everything below here down one
		//console.log('Pop = ' + this.pop.length + ', Score = ' + score + ', Pos = ' + pos + '\n');
		this.pop.splice(pos,0,species);
		if (this.pop.length > this.maxpop) this.pop.pop();
	}
	return pos;
}
population.prototype.iterate = function () {
	var pos1 = Math.floor(Math.random() * (this.maxpop - 2)) + 1;
	var pos2 = Math.floor(Math.random() * pos1);

	var child = this.new_species_factory.childof(this.pop[pos1], this.pop[pos2]);
	var pos = this.insert(child);
	this.iterations++;
}
population.prototype.run = function (complete, args) {
	this.setup(50);
	this.complete = complete;
	this.maxits = 1000000;
	this.mainloop();
}
population.prototype.mainloop = function () {
	for (var i=0; i<1000; i++) {
		if (this.iterations > this.maxits) {
			this.complete();
			process.exit();
		}
		this.iterate();
	}
	that = this;
	setImmediate(function () { that.mainloop(); });
}


// This object creates new properly configured species,
// either by randomization or by breeding.  You could say
// that it knows how species breed and how they are born.
//   species_builder: called to create a configured but empty species
function bridge_breeder(species_builder) {
	this.species_builder = species_builder;
	this.games = 6;
	this.players = 24;
	this.ppert = 4;
	this.maxmutates = 40;
}
// Create a random species
bridge_breeder.prototype.build = function () {
	var out = this.species_builder();
	out.setup(this.games, this.players, this.ppert);
	// Initialize the species to random DNA
	out.randomize();
	return out;
}
// Create a species by breeding
bridge_breeder.prototype.childof = function (p1, p2) {
	var out = this.species_builder();
	out.setup(this.games, this.players, this.ppert);
	for (var i=0; i<this.games; i++) {
		if (Math.random() < 0.5) {
			out.setgame(i, p1.getgame(i));
		} else {
			out.setgame(i, p2.getgame(i));
		}
	}
	var mutates = Math.floor(Math.random() * this.maxmutates);
	for (var i=0; i<mutates; i++) out.mutate();
	return out;
}

function bridge_species() { };
bridge_species.prototype.setup = function (games, players, ppert) {
	this.games = games;
	this.players = players;
	this.ppert = ppert;
	this.dna = [];
	this.score_cache = null;
}
bridge_species.prototype.randomize = function () {
	for (var g=0; g<this.games; g++) this.setgame(g, this.randomgame());
}
bridge_species.prototype.randomgame = function () {
	var player_list = [];
	var out = [];
	for (var i=0; i<this.players; i++) player_list[i] = i;
	// return player_list;
	for (var i=0; i<this.players; i++) {
		var n = Math.floor(Math.random() * player_list.length);
		out[i] = player_list[n];
		player_list.splice(n, 1);
	}
	return out;
}
bridge_species.prototype.setgame = function (i, game) {
	// concat() works like copy, it's important
	this.dna[i] = game.concat();
}
bridge_species.prototype.getgame = function (i) {
	return this.dna[i];
}
bridge_species.prototype.getplayer = function (g,p) {
	return this.dna[g][p];
}
bridge_species.prototype.mutate = function () {
	var t1 = Math.floor(Math.random() * this.players);
	var t2 = Math.floor(Math.random() * this.players);
	if (t1 == t2) return;
	var g = Math.floor(Math.random() * this.games);
	var temp = this.dna[g][t1];
	this.dna[g][t1] = this.dna[g][t2];
	this.dna[g][t2] = temp;
}
bridge_species.prototype.score = function () {
	if (this.score_cache === null) {
		this.score_cache = this.score_partners() + this.score_with();
	}
	return this.score_cache;
}
bridge_species.prototype.score_partners = function (tally) {
	var np = this.players;	// Shortcut
	if (!tally) tally = [];
	for (var i=0; i<np*np; i++) tally[i] = 0;
	var score = 0;
	for (var g=0; g<this.games; g++) {
		for (var pos1=0; pos1<np; pos1+=this.ppert) {
			// Get players at the table
			p1 = this.dna[g][pos1];
			p2 = this.dna[g][pos1+1];
			p3 = this.dna[g][pos1+2];
			p4 = this.dna[g][pos1+3];
			// Get indexes for match-ups at the table
			t1 = p1 * np + p2;
			t2 = p2 * np + p1;
			t3 = p3 * np + p4;
			t4 = p4 * np + p3;
			// Score any match-ups already played
			score += tally[t1] + tally[t2] + tally[t3] + tally[t4];
			// Record the current match-ups
			tally[t1] += 1;
			tally[t2] += 1;
			tally[t3] += 1;
			tally[t4] += 1;
		}
	}
	return score;
}
bridge_species.prototype.score_with = function (tally) {
	var np = this.players;	// Shortcut
	if (!tally) tally = [];
	for (var i=0; i<np*np; i++) tally[i] = 0;
	var score = 0;
	for (var g=0; g<this.games; g++) {
		for (var table=0; table<np; table+=this.ppert) {
			for (var pos1=0; pos1<this.ppert; pos1++) {
				for (var pos2=0; pos2<this.ppert; pos2++) {
					if (pos1 != pos2) {
						var p1 = this.dna[g][table + pos1];
						var p2 = this.dna[g][table + pos2];
						// console.log('Player '
						// + this.player_name(p1) + '-'
						// + this.player_name(p2) + ':'
						// + tally[p1*np+p2]);
						score += tally[p1*np + p2];
						tally[p1*np + p2] += 1;
					}
				}
			}
		}
	}
	return score;
}
bridge_species.prototype.dump_score = function () {
	var np = this.players;	// Shortcut
	var tally1 = [];
	var tally2 = [];
	var score1 = this.score_partners(tally1);
	var score2 = this.score_with(tally2);

	console.log('Scoring Faults: ' + score1 + ' + ' + score2 + ' = ' + this.score());
	var out = ' ';
	for (var pos1=0; pos1<np; pos1++) {
		out += '  ' + this.player_name(pos1);
	}
	console.log(out);
	for (var pos1=0; pos1<np; pos1++) {
		out = this.player_name(pos1);
		for (var pos2=0; pos2<np; pos2++) {
			out += ' ' + tally1[pos1 * np + pos2] + tally2[pos1 * np + pos2];
		}
		console.log(out);
	}
}
bridge_species.prototype.player_name = function (n) {
	return "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789".charAt(n);
}
bridge_species.prototype.toString = function () {
	var out = 'Score: ' + this.score() + '\n';
	for (var g=0; g<this.games; g++) {
		for (var p=0; p<this.players; p++) out += this.player_name(this.getplayer(g,p));
		out += '\n';
	}
	return out;
}
