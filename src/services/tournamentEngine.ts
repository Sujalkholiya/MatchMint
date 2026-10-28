import { Competition, Team, Match, SchedulingConflict } from '../types';

export class TournamentEngine {
  /**
   * Generates single elimination binary tree fixtures
   */
  static generateSingleEliminationFixtures(
    competition: Competition,
    teams: Team[]
  ): Match[] {
    const numTeams = teams.length;
    if (numTeams < 2) return [];

    const sortedTeams = [...teams].sort((a, b) => a.seed - b.seed);
    const roundsCount = Math.ceil(Math.log2(numTeams));
    const totalSlots = Math.pow(2, roundsCount);

    const matches: Match[] = [];
    let matchIdCounter = 1;

    // Standard fold pairing for round 1
    const seedPairs: [number, number][] = [];
    if (totalSlots === 8) {
      seedPairs.push([1, 8], [4, 5], [2, 7], [3, 6]);
    } else if (totalSlots === 4) {
      seedPairs.push([1, 4], [2, 3]);
    } else {
      for (let i = 0; i < totalSlots / 2; i++) {
        seedPairs.push([i + 1, totalSlots - i]);
      }
    }

    const roundMatchMap = new Map<number, Match[]>();

    for (let r = 1; r <= roundsCount; r++) {
      const matchesInRound = Math.pow(2, roundsCount - r);
      roundMatchMap.set(r, []);

      for (let m = 0; m < matchesInRound; m++) {
        let roundName = `Round ${r}`;
        if (r === roundsCount) roundName = 'Final';
        else if (r === roundsCount - 1) roundName = 'Semi-Final';
        else if (r === roundsCount - 2) roundName = 'Quarter-Final';

        const match: Match = {
          id: `match-${competition.id}-r${r}-m${m + 1}`,
          competitionId: competition.id,
          round: r,
          roundName: `${roundName} ${matchesInRound > 1 ? m + 1 : ''}`.trim(),
          matchNumber: matchIdCounter++,
          team1Id: null,
          team2Id: null,
          team1Score: null,
          team2Score: null,
          winnerId: null,
          status: 'upcoming',
          scheduledTime: this.estimateSlotTime(competition.startDate, r, m),
          venue: competition.venues[m % competition.venues.length] || 'Main Stadium',
          nextMatchId: null,
          nextMatchSlot: ((m % 2) + 1) as 1 | 2,
        };
        roundMatchMap.get(r)!.push(match);
        matches.push(match);
      }
    }

    // Link nextMatchId parent pointers
    for (let r = 1; r < roundsCount; r++) {
      const currentMatches = roundMatchMap.get(r)!;
      const nextRoundMatches = roundMatchMap.get(r + 1)!;

      for (let m = 0; m < currentMatches.length; m++) {
        const parentIndex = Math.floor(m / 2);
        currentMatches[m].nextMatchId = nextRoundMatches[parentIndex].id;
      }
    }

    // Populate Round 1 with seeded teams
    const round1Matches = roundMatchMap.get(1)!;
    for (let i = 0; i < seedPairs.length; i++) {
      if (i < round1Matches.length) {
        const [seed1, seed2] = seedPairs[i];
        const team1 = sortedTeams.find(t => t.seed === seed1) || null;
        const team2 = sortedTeams.find(t => t.seed === seed2) || null;
        round1Matches[i].team1Id = team1?.id || null;
        round1Matches[i].team2Id = team2?.id || null;
      }
    }

    return matches;
  }

  /**
   * Generates round robin fixtures using the cyclic algorithm
   */
  static generateRoundRobinFixtures(
    competition: Competition,
    teams: Team[]
  ): Match[] {
    const n = teams.length;
    if (n < 2) return [];

    const teamList = [...teams];
    const isOdd = n % 2 !== 0;
    if (isOdd) {
      teamList.push({
        id: 'BYE',
        competitionId: competition.id,
        name: 'BYE',
        shortTag: 'BYE',
        seed: 999,
        captain: '',
        email: '',
        rosterCount: 0,
        wins: 0,
        draws: 0,
        losses: 0,
        points: 0,
        scoreFor: 0,
        scoreAgainst: 0,
        scoreDiff: 0,
        rank: 999,
        status: 'active',
      });
    }

    const totalRounds = teamList.length - 1;
    const matchesPerRound = teamList.length / 2;
    const matches: Match[] = [];
    let matchCounter = 1;

    for (let round = 1; round <= totalRounds; round++) {
      for (let i = 0; i < matchesPerRound; i++) {
        const t1 = teamList[i];
        const t2 = teamList[teamList.length - 1 - i];

        if (t1.id !== 'BYE' && t2.id !== 'BYE') {
          matches.push({
            id: `match-${competition.id}-rr${round}-m${i + 1}`,
            competitionId: competition.id,
            round,
            roundName: `Round ${round}`,
            matchNumber: matchCounter++,
            team1Id: i % 2 === 0 ? t1.id : t2.id,
            team2Id: i % 2 === 0 ? t2.id : t1.id,
            team1Score: null,
            team2Score: null,
            winnerId: null,
            status: 'upcoming',
            scheduledTime: this.estimateSlotTime(competition.startDate, round, i),
            venue: competition.venues[i % competition.venues.length] || 'Main Stadium',
          });
        }
      }

      // Rotate array keeping index 0 fixed
      const pivot = teamList[0];
      const rest = teamList.slice(1);
      const last = rest.pop()!;
      rest.unshift(last);
      teamList.splice(0, teamList.length, pivot, ...rest);
    }

    return matches;
  }

  /**
   * Recalculates standings based on completed matches
   */
  static recalculateStandings(
    competition: Competition,
    teams: Team[],
    matches: Match[]
  ): Team[] {
    const statsMap = new Map<
      string,
      {
        wins: number;
        draws: number;
        losses: number;
        points: number;
        scoreFor: number;
        scoreAgainst: number;
      }
    >();

    teams.forEach(t => {
      statsMap.set(t.id, {
        wins: 0,
        draws: 0,
        losses: 0,
        points: 0,
        scoreFor: 0,
        scoreAgainst: 0,
      });
    });

    matches
      .filter(m => m.competitionId === competition.id && m.status === 'completed')
      .forEach(m => {
        if (!m.team1Id || !m.team2Id) return;
        const s1 = m.team1Score ?? 0;
        const s2 = m.team2Score ?? 0;

        const t1 = statsMap.get(m.team1Id);
        const t2 = statsMap.get(m.team2Id);

        if (t1) {
          t1.scoreFor += s1;
          t1.scoreAgainst += s2;
          if (s1 > s2) {
            t1.wins += 1;
            t1.points += competition.pointsForWin;
          } else if (s1 < s2) {
            t1.losses += 1;
            t1.points += competition.pointsForLoss;
          } else {
            t1.draws += 1;
            t1.points += competition.pointsForDraw;
          }
        }

        if (t2) {
          t2.scoreFor += s2;
          t2.scoreAgainst += s1;
          if (s2 > s1) {
            t2.wins += 1;
            t2.points += competition.pointsForWin;
          } else if (s2 < s1) {
            t2.losses += 1;
            t2.points += competition.pointsForLoss;
          } else {
            t2.draws += 1;
            t2.points += competition.pointsForDraw;
          }
        }
      });

    const updated = teams.map(team => {
      const stats = statsMap.get(team.id) || {
        wins: 0,
        draws: 0,
        losses: 0,
        points: 0,
        scoreFor: 0,
        scoreAgainst: 0,
      };
      const scoreDiff = stats.scoreFor - stats.scoreAgainst;
      return {
        ...team,
        wins: stats.wins,
        draws: stats.draws,
        losses: stats.losses,
        points: stats.points,
        scoreFor: stats.scoreFor,
        scoreAgainst: stats.scoreAgainst,
        scoreDiff,
      };
    });

    updated.sort((a, b) => {
      if (b.points !== a.points) return b.points - a.points;
      if (b.scoreDiff !== a.scoreDiff) return b.scoreDiff - a.scoreDiff;
      if (b.scoreFor !== a.scoreFor) return b.scoreFor - a.scoreFor;
      return b.wins - a.wins;
    });

    return updated.map((team, idx) => ({
      ...team,
      rank: idx + 1,
    }));
  }

  /**
   * Conflict detection: venue + time overlap only
   */
  static detectSchedulingConflicts(
    competition: Competition,
    matches: Match[]
  ): SchedulingConflict[] {
    const compMatches = matches.filter(
      m => m.competitionId === competition.id
    );
    const conflicts: SchedulingConflict[] = [];
    const durationMs = competition.matchDurationMinutes * 60 * 1000;

    for (let i = 0; i < compMatches.length; i++) {
      for (let j = i + 1; j < compMatches.length; j++) {
        const m1 = compMatches[i];
        const m2 = compMatches[j];

        const t1 = new Date(m1.scheduledTime).getTime();
        const t2 = new Date(m2.scheduledTime).getTime();

        if (isNaN(t1) || isNaN(t2)) continue;

        const m1End = t1 + durationMs;
        const m2End = t2 + durationMs;

        // Check: Same venue and overlapping interval
        if (m1.venue === m2.venue) {
          const isOverlapping = t1 < m2End && m1End > t2;
          if (isOverlapping) {
            conflicts.push({
              type: 'VENUE_COLLISION',
              match1Id: m1.id,
              match2Id: m2.id,
              venue: m1.venue,
              timeSlot: `${m1.scheduledTime} & ${m2.scheduledTime}`,
              description: `Venue clash at "${m1.venue}": Match #${m1.matchNumber} and Match #${m2.matchNumber} overlap.`,
            });
          }
        }
      }
    }

    return conflicts;
  }

  private static estimateSlotTime(
    startDateStr: string,
    roundNumber: number,
    slotIndex: number
  ): string {
    const startHour = 14 + (slotIndex % 3) * 2;
    const hourFormatted = String(startHour).padStart(2, '0');
    return `${startDateStr} ${hourFormatted}:00`;
  }
}
