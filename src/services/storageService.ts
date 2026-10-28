import {
  Competition,
  Team,
  Match,
  HistoryRecord,
  SchedulingConflict,
} from '../types';
import { apiClient } from './apiClient';

export class StorageService {
  /**
   * Retrieves all competitions from C backend
   */
  static async getCompetitions(): Promise<Competition[]> {
    const res = await apiClient.get<Competition[]>('/competitions');
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Retrieves a single competition by ID
   */
  static async getCompetitionById(competitionId: string): Promise<Competition | null> {
    const res = await apiClient.get<Competition>(`/competitions/${competitionId}`);
    if (res.success && res.data) {
      return res.data;
    }
    return null;
  }

  /**
   * Retrieves teams from C backend (optionally filtered by competition)
   */
  static async getTeams(competitionId?: string): Promise<Team[]> {
    const params = competitionId ? { competitionId } : undefined;
    const res = await apiClient.get<Team[]>('/teams', params);
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Retrieves matches from C backend with optional filters
   */
  static async getMatches(
    competitionId?: string,
    status?: string,
    round?: number
  ): Promise<Match[]> {
    const params: Record<string, string | number | undefined> = {};
    if (competitionId) params.competitionId = competitionId;
    if (status) params.status = status;
    if (round) params.round = round;

    const res = await apiClient.get<Match[]>('/matches', params);
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Retrieves history audit logs (LIFO) from C backend
   */
  static async getHistory(competitionId?: string): Promise<HistoryRecord[]> {
    const endpoint = competitionId ? `/competitions/${competitionId}/history` : '/history';
    const res = await apiClient.get<HistoryRecord[]>(endpoint);
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Creates a new tournament in C backend (persisted to SQLite)
   */
  static async createCompetition(data: {
    name: string;
    category: 'sports' | 'esports';
    format: 'single_elimination' | 'round_robin';
    totalTeams: number;
    startDate: string;
    matchDurationMinutes: number;
  }): Promise<Competition> {
    const res = await apiClient.post<Competition>('/competitions', data);
    if (!res.success || !res.data) {
      throw new Error(res.message || 'Failed to create competition on server.');
    }
    return res.data;
  }

  /**
   * Registers a team squad in C backend (persisted to SQLite)
   */
  static async registerTeam(teamData: {
    competitionId: string;
    name: string;
    captain: string;
    rosterCount: number;
    email?: string;
  }): Promise<Team> {
    const res = await apiClient.post<Team>('/teams', teamData);
    if (!res.success || !res.data) {
      throw new Error(res.message || 'Failed to register team on server.');
    }
    return res.data;
  }

  /**
   * Generates fixtures for a competition via C DSA engine
   */
  static async generateFixtures(
    competitionId: string
  ): Promise<{ success: boolean; message: string; count: number }> {
    const res = await apiClient.post<Match[]>(`/competitions/${competitionId}/generate-fixtures`);
    const count = (res as any).count ?? (Array.isArray(res.data) ? res.data.length : 0);
    return {
      success: !!res.success,
      message: res.message || (res.success ? `Fixtures generated successfully.` : 'Failed to generate fixtures.'),
      count,
    };
  }

  static async generateFixturesForCompetition(
    competitionId: string
  ): Promise<{ success: boolean; message: string; count: number }> {
    return this.generateFixtures(competitionId);
  }

  /**
   * Submits match scores, auto-advances winner in binary tree, recalculates standings via C merge sort,
   * and creates an undoable audit snapshot
   */
  static async submitResult(
    matchId: string,
    team1Score: number,
    team2Score: number,
    tieBreakerDetail?: string
  ): Promise<{ success: boolean; message: string }> {
    const res = await apiClient.post(`/matches/${matchId}/result`, {
      team1Score,
      team2Score,
      tieBreakerDetails: tieBreakerDetail || undefined,
    });
    return {
      success: !!res.success,
      message: res.message || (res.success ? 'Result recorded successfully.' : 'Failed to record result.'),
    };
  }

  static async submitMatchResult(
    matchId: string,
    team1Score: number,
    team2Score: number,
    tieBreakerDetail?: string
  ): Promise<{ success: boolean; message: string }> {
    return this.submitResult(matchId, team1Score, team2Score, tieBreakerDetail);
  }

  /**
   * Rolls back match score, clears advanced slots, and recalculates standings safely
   */
  static async rollbackResult(historyId: string): Promise<{ success: boolean; message: string }> {
    const res = await apiClient.post(`/history/${historyId}/undo`);
    return {
      success: !!res.success,
      message: res.message || (res.success ? 'Result successfully undone.' : 'Failed to undo result.'),
    };
  }

  static async undoResult(historyId: string): Promise<{ success: boolean; message: string }> {
    return this.rollbackResult(historyId);
  }

  /**
   * Reschedules a match and checks for venue interval conflicts
   */
  static async rescheduleMatch(
    matchId: string,
    newTime: string,
    newVenue: string
  ): Promise<{ success: boolean; conflicts: SchedulingConflict[]; message?: string }> {
    const res = await apiClient.patch(`/matches/${matchId}/schedule`, {
      scheduledTime: newTime,
      venue: newVenue,
    });

    const conflicts: SchedulingConflict[] =
      (res.data as any)?.conflicts || (res as any).conflicts || [];

    return {
      success: !!res.success,
      conflicts,
      message: res.message,
    };
  }

  /**
   * Checks scheduling conflicts for a competition using C 1D Interval Overlap algorithm
   */
  static async checkConflicts(
    competitionId: string,
    matches?: Match[]
  ): Promise<SchedulingConflict[]> {
    const res = await apiClient.post<SchedulingConflict[]>(
      `/competitions/${competitionId}/check-conflicts`,
      matches ? { matches } : undefined
    );
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return (res as any).conflicts || [];
  }

  /**
   * Retrieves merge-sorted tournament standings from C backend
   */
  static async getStandings(competitionId: string): Promise<Team[]> {
    const res = await apiClient.get<Team[]>(`/competitions/${competitionId}/standings`);
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Retrieves bracket visualization tree from C backend
   */
  static async getBracket(competitionId: string): Promise<any> {
    const res = await apiClient.get(`/competitions/${competitionId}/bracket`);
    return res.data || null;
  }

  /**
   * Retrieves platform dashboard statistical metrics
   */
  static async getDashboardMetrics(): Promise<any> {
    const res = await apiClient.get('/dashboard/metrics');
    return res.data || null;
  }

  /**
   * Retrieves matches scheduled for today
   */
  static async getTodayMatches(): Promise<Match[]> {
    const res = await apiClient.get<Match[]>('/dashboard/today-matches');
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Retrieves distinct venues across all tournaments
   */
  static async getVenues(): Promise<string[]> {
    const res = await apiClient.get<string[]>('/venues');
    if (res.success && Array.isArray(res.data)) {
      return res.data;
    }
    return [];
  }

  /**
   * Exports full SQLite database backup as formatted JSON
   */
  static async exportDatabaseJSON(): Promise<string> {
    const res = await apiClient.get('/settings/backup/export');
    if (res.success && res.data) {
      return JSON.stringify(res.data, null, 2);
    }
    return JSON.stringify(res, null, 2);
  }

  /**
   * Imports JSON backup into SQLite database
   */
  static async importDatabaseJSON(jsonStr: string): Promise<boolean> {
    try {
      const parsed = typeof jsonStr === 'string' ? JSON.parse(jsonStr) : jsonStr;
      const res = await apiClient.post('/settings/backup/import', parsed);
      return !!res.success;
    } catch (err) {
      console.error('[StorageService] importDatabaseJSON parse error:', err);
      return false;
    }
  }

  /**
   * Resets SQLite database back to sample collegiate seed defaults
   */
  static async resetToDefaults(): Promise<boolean> {
    const res = await apiClient.post('/settings/reset');
    return !!res.success;
  }
}
