export type CompetitionCategory = 'sports' | 'esports';

export type TournamentFormat = 'single_elimination' | 'round_robin';

export type CompetitionStatus = 'upcoming' | 'live' | 'completed';

export type MatchStatus = 'upcoming' | 'completed';

export type TeamStatus = 'active' | 'eliminated';

export interface Team {
  id: string;
  competitionId: string;
  name: string;
  shortTag: string;
  seed: number;
  captain: string;
  email: string;
  rosterCount: number;
  wins: number;
  draws: number;
  losses: number;
  points: number;
  scoreFor: number;
  scoreAgainst: number;
  scoreDiff: number;
  rank: number;
  status: TeamStatus;
}

export interface Match {
  id: string;
  competitionId: string;
  round: number;
  roundName: string;
  matchNumber: number;
  team1Id: string | null;
  team2Id: string | null;
  team1Score: number | null;
  team2Score: number | null;
  winnerId: string | null;
  status: MatchStatus;
  scheduledTime: string;
  venue: string;
  nextMatchId?: string | null;
  nextMatchSlot?: 1 | 2;
  tieBreakerDetails?: string;
}

export interface Competition {
  id: string;
  name: string;
  category: CompetitionCategory;
  format: TournamentFormat;
  status: CompetitionStatus;
  description: string;
  totalTeams: number;
  registeredTeamsCount: number;
  currentRound: number;
  totalRounds: number;
  venues: string[];
  matchDurationMinutes: number;
  startDate: string;
  endDate: string;
  pointsForWin: number;
  pointsForDraw: number;
  pointsForLoss: number;
}

export interface HistoryRecord {
  id: string;
  timestamp: string;
  competitionId: string;
  matchId?: string;
  actionType: 'RESULT_ENTERED' | 'RESULT_REVERTED' | 'FIXTURES_GENERATED';
  description: string;
  previousState?: {
    team1Score: number | null;
    team2Score: number | null;
    winnerId: string | null;
    status: MatchStatus;
  };
  canUndo: boolean;
}

export interface SchedulingConflict {
  type: 'VENUE_COLLISION';
  match1Id: string;
  match2Id: string;
  venue: string;
  timeSlot: string;
  description: string;
}
