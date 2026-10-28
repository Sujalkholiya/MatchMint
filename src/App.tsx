import React, { useState, useEffect } from 'react';
import { Competition, Team, Match, HistoryRecord } from './types';
import { StorageService } from './services/storageService';
import { Sidebar, NavItem } from './components/common/Sidebar';
import { Topbar } from './components/common/Topbar';
import { Dashboard } from './pages/Dashboard';
import { Competitions } from './pages/Competitions';
import { CompetitionDetails } from './pages/CompetitionDetails';
import { Teams } from './pages/Teams';
import { Matches } from './pages/Matches';
import { CreateCompetition } from './pages/CreateCompetition';
import { Settings } from './pages/Settings';
import { Landing } from './pages/Landing';
import { EnterResultModal } from './components/competition/EnterResultModal';
import { ScheduleMatchModal } from './components/competition/ScheduleMatchModal';
import { RegisterTeamModal } from './components/competition/RegisterTeamModal';
import {
  LayoutDashboard,
  Trophy,
  Swords,
  Users,
  Settings as SettingsIcon,
  CheckCircle2,
  AlertCircle,
  X,
} from 'lucide-react';

export function App() {
  // Navigation & View Mode ('landing' vs 'app')
  const [viewMode, setViewMode] = useState<'landing' | 'app'>(() => {
    if (typeof window !== 'undefined') {
      return window.location.pathname.startsWith('/app') ? 'app' : 'landing';
    }
    return 'landing';
  });

  const [competitions, setCompetitions] = useState<Competition[]>([]);
  const [teams, setTeams] = useState<Team[]>([]);
  const [matches, setMatches] = useState<Match[]>([]);
  const [history, setHistory] = useState<HistoryRecord[]>([]);

  const [currentTab, setCurrentTab] = useState<NavItem | 'create_competition' | 'competition_details'>('dashboard');
  const [selectedCompetition, setSelectedCompetition] = useState<Competition | null>(null);

  // Modals state
  const [resultModalMatch, setResultModalMatch] = useState<Match | null>(null);
  const [scheduleModalMatch, setScheduleModalMatch] = useState<Match | null>(null);
  const [isRegisterTeamOpen, setIsRegisterTeamOpen] = useState(false);

  // Toast
  const [toast, setToast] = useState<{ message: string; type: 'success' | 'error' | 'info' } | null>(null);

  const reloadData = async () => {
    try {
      const [c, t, m, h] = await Promise.all([
        StorageService.getCompetitions(),
        StorageService.getTeams(),
        StorageService.getMatches(),
        StorageService.getHistory(),
      ]);

      setCompetitions(c);
      setTeams(t);
      setMatches(m);
      setHistory(h);

      if (selectedCompetition) {
        const refreshed = c.find((comp) => comp.id === selectedCompetition.id);
        if (refreshed) setSelectedCompetition(refreshed);
      }
    } catch (err) {
      console.error('[MatchMint] Failed to load data from backend:', err);
    }
  };

  useEffect(() => {
    reloadData();

    // Listen to browser back/forward buttons
    const handlePopState = () => {
      const isApp = window.location.pathname.startsWith('/app');
      setViewMode(isApp ? 'app' : 'landing');
    };

    window.addEventListener('popstate', handlePopState);
    return () => window.removeEventListener('popstate', handlePopState);
  }, []);

  const showToast = (message: string, type: 'success' | 'error' | 'info' = 'success') => {
    setToast({ message, type });
    setTimeout(() => {
      setToast(null);
    }, 3500);
  };

  const navigateToApp = (tab?: string, competitionId?: string) => {
    setViewMode('app');
    if (window.location.pathname !== '/app') {
      window.history.pushState({}, '', '/app');
    }
    if (competitionId) {
      const c = competitions.find((comp) => comp.id === competitionId);
      if (c) {
        setSelectedCompetition(c);
        setCurrentTab('competition_details');
        return;
      }
    }
    if (tab) {
      setCurrentTab(tab as any);
    }
  };

  const navigateToLanding = () => {
    setViewMode('landing');
    if (window.location.pathname !== '/') {
      window.history.pushState({}, '', '/');
    }
    window.scrollTo({ top: 0, behavior: 'smooth' });
  };

  const handleSelectCompetition = (comp: Competition) => {
    setSelectedCompetition(comp);
    setCurrentTab('competition_details');
  };

  const handleOpenResultModal = (match: Match) => {
    setResultModalMatch(match);
  };

  const handleOpenScheduleModal = (match: Match) => {
    setScheduleModalMatch(match);
  };

  const handleEnterResultSubmit = async (
    matchId: string,
    team1Score: number,
    team2Score: number,
    tieBreakerDetail?: string
  ) => {
    const res = await StorageService.submitResult(matchId, team1Score, team2Score, tieBreakerDetail);
    if (res.success) {
      await reloadData();
      showToast(res.message, 'success');
    } else {
      showToast(res.message, 'error');
    }
  };

  const handleUndoResult = async (historyId: string) => {
    const res = await StorageService.rollbackResult(historyId);
    if (res.success) {
      await reloadData();
      showToast(res.message, 'success');
    } else {
      showToast(res.message, 'error');
    }
  };

  const handleGenerateFixtures = async (comp: Competition) => {
    const res = await StorageService.generateFixturesForCompetition(comp.id);
    if (res.success) {
      await reloadData();
      showToast(res.message, 'success');
    } else {
      showToast(res.message, 'error');
    }
  };

  const getPageTitle = () => {
    switch (currentTab) {
      case 'dashboard':
        return 'Dashboard';
      case 'competitions':
        return 'Competitions';
      case 'competition_details':
        return selectedCompetition?.name || 'Tournament Details';
      case 'teams':
        return 'Teams';
      case 'matches':
        return 'Matches';
      case 'create_competition':
        return 'Create Competition';
      case 'settings':
        return 'Settings';
      default:
        return 'MatchMint';
    }
  };

  const getBreadcrumb = () => {
    if (currentTab === 'competition_details' && selectedCompetition) {
      return `Competitions / ${selectedCompetition.name}`;
    }
    if (currentTab === 'create_competition') {
      return 'Competitions / Create';
    }
    return undefined;
  };

  // -------------------------------------------------------------
  // LANDING PAGE VIEW ("/")
  // -------------------------------------------------------------
  if (viewMode === 'landing') {
    return <Landing onOpenApp={navigateToApp} competitions={competitions} />;
  }

  // -------------------------------------------------------------
  // APP VIEW ("/app")
  // -------------------------------------------------------------
  return (
    <div className="min-h-screen bg-[#0C0C0C] text-[#D7E2EA] flex flex-col md:flex-row antialiased font-sans selection:bg-[#B600A8]/30 selection:text-white">
      {/* Toast Notification */}
      {toast && (
        <div className="fixed top-4 right-4 z-50 flex items-center gap-2.5 px-4 py-2.5 rounded-full bg-[#12151a] border border-[#B600A8]/40 shadow-2xl text-xs font-mono animate-in slide-in-from-top-2 duration-150">
          {toast.type === 'success' ? (
            <CheckCircle2 className="w-4 h-4 text-[#B600A8] shrink-0" />
          ) : (
            <AlertCircle className="w-4 h-4 text-rose-400 shrink-0" />
          )}
          <span className="text-white">{toast.message}</span>
          <button
            onClick={() => setToast(null)}
            className="text-[#D7E2EA]/60 hover:text-white ml-2 cursor-pointer"
          >
            <X className="w-3.5 h-3.5" />
          </button>
        </div>
      )}

      {/* Desktop Persistent Left Sidebar (5 items) */}
      <div className="hidden md:block shrink-0">
        <Sidebar
          currentTab={
            currentTab === 'competition_details' || currentTab === 'create_competition'
              ? 'competitions'
              : currentTab
          }
          onSelectTab={(tab) => {
            setCurrentTab(tab);
            if (tab !== 'competitions') setSelectedCompetition(null);
          }}
          onCreateCompetition={() => setCurrentTab('create_competition')}
          onLogoClick={navigateToLanding}
        />
      </div>

      {/* Main Workspace Column */}
      <div className="flex-1 flex flex-col min-w-0 min-h-screen bg-[#0C0C0C]">
        {/* Topbar */}
        <Topbar
          title={getPageTitle()}
          subtitle={
            currentTab === 'dashboard'
              ? 'Tournament overview and match schedule.'
              : undefined
          }
          breadcrumb={getBreadcrumb()}
          primaryAction={
            currentTab === 'competitions' || currentTab === 'dashboard'
              ? {
                  label: 'Create Tournament',
                  onClick: () => setCurrentTab('create_competition'),
                }
              : currentTab === 'teams'
              ? {
                  label: 'Add Team',
                  onClick: () => setIsRegisterTeamOpen(true),
                }
              : undefined
          }
        />

        {/* Content Body Router */}
        <main className="flex-1 overflow-y-auto">
          {currentTab === 'dashboard' && (
            <Dashboard
              competitions={competitions}
              teams={teams}
              matches={matches}
              onSelectCompetition={handleSelectCompetition}
              onOpenResultModal={handleOpenResultModal}
            />
          )}

          {currentTab === 'competitions' && (
            <Competitions
              competitions={competitions}
              teams={teams}
              matches={matches}
              onSelectCompetition={handleSelectCompetition}
              onCreateCompetition={() => setCurrentTab('create_competition')}
            />
          )}

          {currentTab === 'competition_details' && selectedCompetition && (
            <CompetitionDetails
              competition={selectedCompetition}
              teams={teams}
              matches={matches}
              history={history}
              onBack={() => setCurrentTab('competitions')}
              onOpenResultModal={handleOpenResultModal}
              onGenerateFixtures={handleGenerateFixtures}
              onUndoResult={handleUndoResult}
            />
          )}

          {currentTab === 'teams' && (
            <Teams
              teams={teams}
              competitions={competitions}
              onOpenRegisterModal={() => setIsRegisterTeamOpen(true)}
              onSelectCompetition={handleSelectCompetition}
            />
          )}

          {currentTab === 'matches' && (
            <Matches
              matches={matches}
              teams={teams}
              competitions={competitions}
              history={history}
              onOpenResultModal={handleOpenResultModal}
              onOpenScheduleModal={handleOpenScheduleModal}
              onUndoResult={handleUndoResult}
            />
          )}

          {currentTab === 'create_competition' && (
            <CreateCompetition
              onCreated={async (newComp) => {
                await reloadData();
                handleSelectCompetition(newComp);
                showToast(`Tournament "${newComp.name}" created.`, 'success');
              }}
              onCancel={() => setCurrentTab('competitions')}
            />
          )}

          {currentTab === 'settings' && (
            <Settings onResetData={reloadData} />
          )}
        </main>

        {/* Mobile Navigation Bar (5 tabs) */}
        <nav className="md:hidden border-t border-[#D7E2EA]/15 bg-[#12151a] px-2 py-2 flex items-center justify-around sticky bottom-0 z-30 text-[11px] font-mono">
          <button
            onClick={() => setCurrentTab('dashboard')}
            className={`p-1.5 rounded-lg flex flex-col items-center gap-1 cursor-pointer ${
              currentTab === 'dashboard' ? 'text-[#B600A8] font-bold' : 'text-[#D7E2EA]/60'
            }`}
          >
            <LayoutDashboard className="w-4 h-4" />
            <span>Dash</span>
          </button>
          <button
            onClick={() => setCurrentTab('competitions')}
            className={`p-1.5 rounded-lg flex flex-col items-center gap-1 cursor-pointer ${
              currentTab === 'competitions' || currentTab === 'competition_details'
                ? 'text-[#B600A8] font-bold'
                : 'text-[#D7E2EA]/60'
            }`}
          >
            <Trophy className="w-4 h-4" />
            <span>Tournaments</span>
          </button>
          <button
            onClick={() => setCurrentTab('matches')}
            className={`p-1.5 rounded-lg flex flex-col items-center gap-1 cursor-pointer ${
              currentTab === 'matches' ? 'text-[#B600A8] font-bold' : 'text-[#D7E2EA]/60'
            }`}
          >
            <Swords className="w-4 h-4" />
            <span>Matches</span>
          </button>
          <button
            onClick={() => setCurrentTab('teams')}
            className={`p-1.5 rounded-lg flex flex-col items-center gap-1 cursor-pointer ${
              currentTab === 'teams' ? 'text-[#B600A8] font-bold' : 'text-[#D7E2EA]/60'
            }`}
          >
            <Users className="w-4 h-4" />
            <span>Teams</span>
          </button>
          <button
            onClick={() => setCurrentTab('settings')}
            className={`p-1.5 rounded-lg flex flex-col items-center gap-1 cursor-pointer ${
              currentTab === 'settings' ? 'text-[#B600A8] font-bold' : 'text-[#D7E2EA]/60'
            }`}
          >
            <SettingsIcon className="w-4 h-4" />
            <span>Settings</span>
          </button>
        </nav>
      </div>

      {/* Global Modals */}
      <EnterResultModal
        isOpen={!!resultModalMatch}
        onClose={() => setResultModalMatch(null)}
        match={resultModalMatch}
        teams={teams}
        competition={
          competitions.find((c) => c.id === resultModalMatch?.competitionId) || null
        }
        onSubmit={handleEnterResultSubmit}
      />

      <ScheduleMatchModal
        isOpen={!!scheduleModalMatch}
        onClose={() => setScheduleModalMatch(null)}
        match={scheduleModalMatch}
        competition={
          competitions.find((c) => c.id === scheduleModalMatch?.competitionId) || null
        }
        onScheduled={() => {
          reloadData();
          showToast('Match schedule updated.', 'success');
        }}
      />

      <RegisterTeamModal
        isOpen={isRegisterTeamOpen}
        onClose={() => setIsRegisterTeamOpen(false)}
        competitions={competitions}
        initialCompetitionId={selectedCompetition?.id}
        onRegistered={() => {
          reloadData();
          showToast('Team registered successfully.', 'success');
        }}
      />
    </div>
  );
}

export default App;
