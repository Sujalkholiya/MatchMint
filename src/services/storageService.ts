import {
  Competition,
  Team,
  Match,
  HistoryRecord,
  SchedulingConflict,
} from '../types';
import {
  INITIAL_COMPETITIONS,
  INITIAL_TEAMS,
  INITIAL_MATCHES,
  INITIAL_HISTORY,
} from '../data/mockData';
import { TournamentEngine } from './tournamentEngine';

const STORAGE_KEY_PREFIX = 'matchmint_v2_';

export class StorageService {
  private static initCheck(): void {
    const initialized = localStorage.getItem(`${STORAGE_KEY_PREFIX}initialized`);
    if (!initialized) {
      this.resetToDefaults();
      localStorage.setItem(`${STORAGE_KEY_PREFIX}initialized`, 'true');
    }
  }

  static getCompetitions(): Competition[] {
    this.initCheck();
    const raw = localStorage.getItem(`${STORAGE_KEY_PREFIX}competitions`);
    if (!raw) return INITIAL_COMPETITIONS;
    try {
      return JSON.parse(raw);
    } catch {
      return INITIAL_COMPETITIONS;
    }
  }

  static saveCompetitions(competitions: Competition[]): void {
    localStorage.setItem(`${STORAGE_KEY_PREFIX}competitions`, JSON.stringify(competitions));
  }

  static getTeams(competitionId?: string): Team[] {
    this.initCheck();
    const raw = localStorage.getItem(`${STORAGE_KEY_PREFIX}teams`);
    let teams = INITIAL_TEAMS;
    if (raw) {
      try {
        teams = JSON.parse(raw);
      } catch {
        teams = INITIAL_TEAMS;
      }
    }
    if (competitionId) {
      return teams.filter(t => t.competitionId === competitionId);
    }
    return teams;
  }

  static saveTeams(teams: Team[]): void {
    localStorage.setItem(`${STORAGE_KEY_PREFIX}teams`, JSON.stringify(teams));
  }

  static getMatches(): Match[] {
    this.initCheck();
    const raw = localStorage.getItem(`${STORAGE_KEY_PREFIX}matches`);
    if (!raw) return INITIAL_MATCHES;
    try {
      return JSON.parse(raw);
    } catch {
      return INITIAL_MATCHES;
    }
  }

  static saveMatches(matches: Match[]): void {
    localStorage.setItem(`${STORAGE_KEY_PREFIX}matches`, JSON.stringify(matches));
  }

  static getHistory(): HistoryRecord[] {
    this.initCheck();
    const raw = localStorage.getItem(`${STORAGE_KEY_PREFIX}history`);
    if (!raw) return INITIAL_HISTORY;
    try {
      return JSON.parse(raw);
    } catch {
      return INITIAL_HISTORY;
    }
  }

  static saveHistory(history: HistoryRecord[]): void {
    // Keep only last 20 events
    const trimmed = history.slice(0, 20);
    localStorage.setItem(`${STORAGE_KEY_PREFIX}history`, JSON.stringify(trimmed));
  }

  static resetToDefaults(): void {
    this.saveCompetitions(INITIAL_COMPETITIONS);
    this.saveTeams(INITIAL_TEAMS);
    this.saveMatches(INITIAL_MATCHES);
    this.saveHistory(INITIAL_HISTORY);
  }

  static submitMatchResult(
    matchId: string,
    team1Score: number,
    team2Score: number,
    tieBreakerDetail?: string
  ): { success: boolean; message: string } {
    const matches = this.getMatches();
    const teams = this.getTeams();
    const competitions = this.getCompetitions();
    const history = this.getHistory();

    const matchIndex = matches.findIndex(m => m.id === matchId);
    if (matchIndex === -1) return { success: false, message: 'Match not found.' };

    const match = matches[matchIndex];
    const comp = competitions.find(c => c.id === match.competitionId);
    if (!comp) return { success: false, message: 'Competition not found.' };

    const team1 = teams.find(t => t.id === match.team1Id);
    const team2 = teams.find(t => t.id === match.team2Id);

    const previousState = {
      team1Score: match.team1Score,
      team2Score: match.team2Score,
      winnerId: match.winnerId,
      status: match.status,
    };

    let winnerId: string | null = null;
    if (team1Score > team2Score) {
      winnerId = match.team1Id;
    } else if (team2Score > team1Score) {
      winnerId = match.team2Id;
    } else {
      if (comp.format === 'single_elimination') {
        winnerId = match.team1Id;
      } else {
        winnerId = null;
      }
    }

    matches[matchIndex] = {
      ...match,
      team1Score,
      team2Score,
      winnerId,
      status: 'completed',
      tieBreakerDetails: tieBreakerDetail || undefined,
    };

    // Auto-advance winner in single-elimination
    if (match.nextMatchId && winnerId) {
      const nextMatchIndex = matches.findIndex(m => m.id === match.nextMatchId);
      if (nextMatchIndex !== -1) {
        const nextMatch = matches[nextMatchIndex];
        if (match.nextMatchSlot === 1) {
          matches[nextMatchIndex] = { ...nextMatch, team1Id: winnerId };
        } else if (match.nextMatchSlot === 2) {
          matches[nextMatchIndex] = { ...nextMatch, team2Id: winnerId };
        }
      }
    }

    const updatedTeams = TournamentEngine.recalculateStandings(comp, teams, matches);

    const t1Name = team1 ? team1.name : 'TBD';
    const t2Name = team2 ? team2.name : 'TBD';
    const newRecord: HistoryRecord = {
      id: `hist-${Date.now()}`,
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
      competitionId: comp.id,
      matchId: match.id,
      actionType: 'RESULT_ENTERED',
      description: `${t1Name} (${team1Score}) vs (${team2Score}) ${t2Name}`,
      previousState,
      canUndo: true,
    };

    // Disable undo on earlier results for this match
    const updatedHistory = history.map(h => (h.matchId === match.id ? { ...h, canUndo: false } : h));
    updatedHistory.unshift(newRecord);

    this.saveMatches(matches);
    this.saveTeams(updatedTeams);
    this.saveHistory(updatedHistory);

    return { success: true, message: `Result recorded: ${t1Name} ${team1Score} — ${team2Score} ${t2Name}` };
  }

  static undoResult(historyId: string): { success: boolean; message: string } {
    const history = this.getHistory();
    const matches = this.getMatches();
    const teams = this.getTeams();
    const competitions = this.getCompetitions();

    const recordIndex = history.findIndex(h => h.id === historyId);
    if (recordIndex === -1) return { success: false, message: 'History record not found.' };

    const record = history[recordIndex];
    if (!record.canUndo || !record.matchId || !record.previousState) {
      return { success: false, message: 'This result cannot be undone.' };
    }

    const matchIndex = matches.findIndex(m => m.id === record.matchId);
    if (matchIndex === -1) return { success: false, message: 'Target match not found.' };

    const currentMatch = matches[matchIndex];
    const comp = competitions.find(c => c.id === currentMatch.competitionId);
    if (!comp) return { success: false, message: 'Competition not found.' };

    // Check if downstream match completed
    if (currentMatch.nextMatchId) {
      const nextMatch = matches.find(m => m.id === currentMatch.nextMatchId);
      if (nextMatch && nextMatch.status === 'completed') {
        return {
          success: false,
          message: `Cannot undo: Next round match has already been played.`,
        };
      }
      // Clear advanced slot
      const nextIndex = matches.findIndex(m => m.id === currentMatch.nextMatchId);
      if (nextIndex !== -1) {
        const nextM = matches[nextIndex];
        if (currentMatch.nextMatchSlot === 1 && nextM.team1Id === currentMatch.winnerId) {
          matches[nextIndex] = { ...nextM, team1Id: null };
        } else if (currentMatch.nextMatchSlot === 2 && nextM.team2Id === currentMatch.winnerId) {
          matches[nextIndex] = { ...nextM, team2Id: null };
        }
      }
    }

    // Roll back match
    matches[matchIndex] = {
      ...currentMatch,
      team1Score: record.previousState.team1Score,
      team2Score: record.previousState.team2Score,
      winnerId: record.previousState.winnerId,
      status: record.previousState.status,
      tieBreakerDetails: undefined,
    };

    const updatedTeams = TournamentEngine.recalculateStandings(comp, teams, matches);

    const revertAudit: HistoryRecord = {
      id: `hist-revert-${Date.now()}`,
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
      competitionId: comp.id,
      matchId: currentMatch.id,
      actionType: 'RESULT_REVERTED',
      description: `Reverted result for ${currentMatch.roundName}`,
      canUndo: false,
    };

    history[recordIndex].canUndo = false;
    history.unshift(revertAudit);

    this.saveMatches(matches);
    this.saveTeams(updatedTeams);
    this.saveHistory(history);

    return {
      success: true,
      message: 'Result successfully undone.',
    };
  }

  static generateFixtures(competitionId: string): { success: boolean; message: string; count: number } {
    const competitions = this.getCompetitions();
    const compIndex = competitions.findIndex(c => c.id === competitionId);
    if (compIndex === -1) return { success: false, message: 'Competition not found', count: 0 };

    const comp = competitions[compIndex];
    const allTeams = this.getTeams().filter(t => t.competitionId === competitionId);

    if (allTeams.length < 2) {
      return { success: false, message: 'At least 2 teams are required to generate fixtures.', count: 0 };
    }

    let generated: Match[] = [];
    if (comp.format === 'round_robin') {
      generated = TournamentEngine.generateRoundRobinFixtures(comp, allTeams);
    } else {
      generated = TournamentEngine.generateSingleEliminationFixtures(comp, allTeams);
    }

    const existingOtherMatches = this.getMatches().filter(m => m.competitionId !== competitionId);
    const updatedMatches = [...existingOtherMatches, ...generated];

    competitions[compIndex] = {
      ...comp,
      status: 'live',
      currentRound: 1,
    };

    const history = this.getHistory();
    history.unshift({
      id: `hist-gen-${Date.now()}`,
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
      competitionId: comp.id,
      actionType: 'FIXTURES_GENERATED',
      description: `Generated ${generated.length} fixtures (${comp.format === 'round_robin' ? 'Round Robin' : 'Single Elimination'})`,
      canUndo: false,
    });

    this.saveMatches(updatedMatches);
    this.saveCompetitions(competitions);
    this.saveHistory(history);

    return {
      success: true,
      message: `Generated ${generated.length} fixtures for ${comp.name}.`,
      count: generated.length,
    };
  }

  static submitResult(
    matchId: string,
    team1Score: number,
    team2Score: number,
    tieBreakerDetail?: string
  ) {
    return this.submitMatchResult(matchId, team1Score, team2Score, tieBreakerDetail);
  }

  static rollbackResult(historyId: string) {
    return this.undoResult(historyId);
  }

  static generateFixturesForCompetition(competitionId: string) {
    return this.generateFixtures(competitionId);
  }

  static registerTeam(teamData: {
    competitionId: string;
    name: string;
    captain: string;
    rosterCount: number;
    email?: string;
  }): Team {
    const teams = this.getTeams();
    const compTeams = teams.filter(t => t.competitionId === teamData.competitionId);

    const tag = teamData.name.trim().substring(0, 3).toUpperCase();
    const newTeam: Team = {
      id: `team-${Date.now()}`,
      competitionId: teamData.competitionId,
      name: teamData.name.trim(),
      shortTag: tag,
      seed: compTeams.length + 1,
      captain: teamData.captain.trim(),
      email: teamData.email || `${tag.toLowerCase()}@campus.edu`,
      rosterCount: teamData.rosterCount || 5,
      wins: 0,
      draws: 0,
      losses: 0,
      points: 0,
      scoreFor: 0,
      scoreAgainst: 0,
      scoreDiff: 0,
      rank: compTeams.length + 1,
      status: 'active',
    };

    teams.push(newTeam);
    this.saveTeams(teams);

    const comps = this.getCompetitions();
    const cIndex = comps.findIndex(c => c.id === teamData.competitionId);
    if (cIndex !== -1) {
      comps[cIndex].registeredTeamsCount = compTeams.length + 1;
      this.saveCompetitions(comps);
    }

    return newTeam;
  }

  static createCompetition(data: {
    name: string;
    category: 'sports' | 'esports';
    format: 'single_elimination' | 'round_robin';
    totalTeams: number;
    startDate: string;
    matchDurationMinutes: number;
  }): Competition {
    const comps = this.getCompetitions();
    const rounds =
      data.format === 'single_elimination'
        ? Math.ceil(Math.log2(data.totalTeams))
        : data.totalTeams - 1;

    const newComp: Competition = {
      ...data,
      id: `comp-${Date.now()}`,
      status: 'upcoming',
      description: `${data.category === 'sports' ? 'Sports' : 'Esports'} competition. Format: ${data.format === 'round_robin' ? 'Round Robin' : 'Single Elimination'}.`,
      registeredTeamsCount: 0,
      currentRound: 0,
      totalRounds: rounds,
      venues: ['Main Stadium', 'Arena 2'],
      endDate: data.startDate,
      pointsForWin: 3,
      pointsForDraw: 1,
      pointsForLoss: 0,
    };

    comps.unshift(newComp);
    this.saveCompetitions(comps);
    return newComp;
  }

  static rescheduleMatch(
    matchId: string,
    newTime: string,
    newVenue: string
  ): { success: boolean; conflicts: SchedulingConflict[] } {
    const matches = this.getMatches();
    const matchIndex = matches.findIndex(m => m.id === matchId);
    if (matchIndex === -1) return { success: false, conflicts: [] };

    const comp = this.getCompetitions().find(c => c.id === matches[matchIndex].competitionId);
    if (!comp) return { success: false, conflicts: [] };

    matches[matchIndex].scheduledTime = newTime;
    matches[matchIndex].venue = newVenue;

    const conflicts = TournamentEngine.detectSchedulingConflicts(comp, matches);
    this.saveMatches(matches);
    return { success: true, conflicts };
  }

  static exportDatabaseJSON(): string {
    const data = {
      competitions: this.getCompetitions(),
      teams: this.getTeams(),
      matches: this.getMatches(),
      history: this.getHistory(),
    };
    return JSON.stringify(data, null, 2);
  }

  static importDatabaseJSON(jsonStr: string): boolean {
    try {
      const parsed = JSON.parse(jsonStr);
      if (parsed.competitions && parsed.teams && parsed.matches) {
        this.saveCompetitions(parsed.competitions);
        this.saveTeams(parsed.teams);
        this.saveMatches(parsed.matches);
        if (parsed.history) this.saveHistory(parsed.history);
        return true;
      }
      return false;
    } catch {
      return false;
    }
  }
}
