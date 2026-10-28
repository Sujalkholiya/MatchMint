/**
 * MatchMint Centralized REST API Client
 * Connects React Frontend to High-Performance C11 Backend Server
 */

export interface ApiResponse<T = any> {
  success: boolean;
  message?: string;
  data?: T;
  error?: string;
  count?: number;
  conflicts?: any[];
}

const getApiBaseUrl = (): string => {
  const envUrl = import.meta.env.VITE_API_BASE_URL;
  if (envUrl && typeof envUrl === 'string' && envUrl.trim().length > 0) {
    return envUrl.replace(/\/+$/, '');
  }
  return 'http://localhost:8080/api/v1';
};

class ApiClient {
  private getHeaders(): Record<string, string> {
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      'Accept': 'application/json',
    };

    try {
      const token = localStorage.getItem('matchmint_token');
      if (token) {
        headers['Authorization'] = `Bearer ${token}`;
      }
    } catch {
      // Ignore localStorage access restrictions if any
    }

    return headers;
  }

  private buildUrl(endpoint: string, params?: Record<string, string | number | boolean | undefined | null>): string {
    const base = getApiBaseUrl();
    const cleanEndpoint = endpoint.startsWith('/') ? endpoint : `/${endpoint}`;
    let url = `${base}${cleanEndpoint}`;

    if (params) {
      const queryParams = new URLSearchParams();
      Object.entries(params).forEach(([key, val]) => {
        if (val !== undefined && val !== null && val !== '') {
          queryParams.append(key, String(val));
        }
      });
      const qs = queryParams.toString();
      if (qs.length > 0) {
        url += (url.includes('?') ? '&' : '?') + qs;
      }
    }

    return url;
  }

  async get<T = any>(
    endpoint: string,
    params?: Record<string, string | number | boolean | undefined | null>
  ): Promise<ApiResponse<T>> {
    try {
      const url = this.buildUrl(endpoint, params);
      const res = await fetch(url, {
        method: 'GET',
        headers: this.getHeaders(),
      });

      const json = await res.json();
      return json;
    } catch (err: any) {
      console.warn(`[ApiClient] GET ${endpoint} failed:`, err);
      return {
        success: false,
        message: err?.message || 'Network connection failed to backend server.',
      };
    }
  }

  async post<T = any>(endpoint: string, body?: any): Promise<ApiResponse<T>> {
    try {
      const url = this.buildUrl(endpoint);
      const res = await fetch(url, {
        method: 'POST',
        headers: this.getHeaders(),
        body: body !== undefined ? JSON.stringify(body) : undefined,
      });

      const json = await res.json();
      return json;
    } catch (err: any) {
      console.warn(`[ApiClient] POST ${endpoint} failed:`, err);
      return {
        success: false,
        message: err?.message || 'Network connection failed to backend server.',
      };
    }
  }

  async put<T = any>(endpoint: string, body?: any): Promise<ApiResponse<T>> {
    try {
      const url = this.buildUrl(endpoint);
      const res = await fetch(url, {
        method: 'PUT',
        headers: this.getHeaders(),
        body: body !== undefined ? JSON.stringify(body) : undefined,
      });

      const json = await res.json();
      return json;
    } catch (err: any) {
      console.warn(`[ApiClient] PUT ${endpoint} failed:`, err);
      return {
        success: false,
        message: err?.message || 'Network connection failed to backend server.',
      };
    }
  }

  async patch<T = any>(endpoint: string, body?: any): Promise<ApiResponse<T>> {
    try {
      const url = this.buildUrl(endpoint);
      const res = await fetch(url, {
        method: 'PATCH',
        headers: this.getHeaders(),
        body: body !== undefined ? JSON.stringify(body) : undefined,
      });

      const json = await res.json();
      return json;
    } catch (err: any) {
      console.warn(`[ApiClient] PATCH ${endpoint} failed:`, err);
      return {
        success: false,
        message: err?.message || 'Network connection failed to backend server.',
      };
    }
  }

  async delete<T = any>(endpoint: string): Promise<ApiResponse<T>> {
    try {
      const url = this.buildUrl(endpoint);
      const res = await fetch(url, {
        method: 'DELETE',
        headers: this.getHeaders(),
      });

      const json = await res.json();
      return json;
    } catch (err: any) {
      console.warn(`[ApiClient] DELETE ${endpoint} failed:`, err);
      return {
        success: false,
        message: err?.message || 'Network connection failed to backend server.',
      };
    }
  }
}

export const apiClient = new ApiClient();
